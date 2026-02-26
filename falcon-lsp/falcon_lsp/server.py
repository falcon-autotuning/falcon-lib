"""pygls-based LSP server for the Falcon autotuner DSL."""

from __future__ import annotations

import logging
import re
from typing import Optional

from lsprotocol import types as lsp

from pygls.server import LanguageServer

from .ast_nodes import (
    AutotunerDecl,
    CallExpr,
    Expr,
    MemberExpr,
    MethodCallExpr,
    Program,
    StateDecl,
    TypeDescriptor,
    VarExpr,
)
from .builtins import DEFAULT_REGISTRY
from .parser import ParseError, parse
from .type_checker import Diagnostic, TypeChecker

logger = logging.getLogger(__name__)

# ---------------------------------------------------------------------------
# Server helpers
# ---------------------------------------------------------------------------

SERVER_NAME = "falcon-lsp"
SERVER_VERSION = "0.1.0"


class _DocumentState:
    """Cache entry for a single open document."""

    def __init__(
        self,
        text: str,
        program: Optional[Program],
        parse_errors: list[ParseError],
        diagnostics: list[Diagnostic],
    ) -> None:
        self.text = text
        self.program = program
        self.parse_errors = parse_errors
        self.diagnostics = diagnostics


def _to_lsp_diag(err: ParseError | Diagnostic) -> lsp.Diagnostic:
    """Convert a parse error or type diagnostic to an LSP Diagnostic."""
    if isinstance(err, ParseError):
        line = max(0, err.line - 1)
        col = max(0, err.col - 1)
        sev = lsp.DiagnosticSeverity.Error
        msg = err.message
    else:
        line = max(0, err.line - 1)
        col = max(0, err.col - 1)
        sev = (
            lsp.DiagnosticSeverity.Error
            if err.severity == "error"
            else lsp.DiagnosticSeverity.Warning
        )
        msg = err.message

    return lsp.Diagnostic(
        range=lsp.Range(
            start=lsp.Position(line=line, character=col),
            end=lsp.Position(line=line, character=col + 1),
        ),
        message=msg,
        severity=sev,
        source=SERVER_NAME,
    )


def _analyse(text: str) -> _DocumentState:
    """Parse + type-check *text* and return a document state."""
    program, parse_errors = parse(text)
    diagnostics: list[Diagnostic] = []
    if program:
        checker = TypeChecker()
        diagnostics = checker.check(program)
    return _DocumentState(text, program, parse_errors, diagnostics)


# ---------------------------------------------------------------------------
# Symbol helpers (for hover / goto-def)
# ---------------------------------------------------------------------------

def _collect_symbols(
    program: Optional[Program],
) -> dict[str, tuple[str, int, int]]:
    """Return a flat map: name -> (hover_text, line, col)."""
    syms: dict[str, tuple[str, int, int]] = {}
    if program is None:
        return syms

    for at in program.autotuners:
        syms[at.name] = (f"(autotuner) {at.name}", at.line, at.column)
        for p in at.input_params:
            syms[f"{at.name}.in.{p.name}"] = (
                f"(input param) {p.name}: {p.type}", p.line, p.column,
            )
        for p in at.output_params:
            syms[f"{at.name}.out.{p.name}"] = (
                f"(output param) {p.name}: {p.type}", p.line, p.column,
            )
        for state in at.states:
            syms[f"{at.name}.state.{state.name}"] = (
                f"(state) {state.name}", state.line, state.column,
            )

    for rt in program.routines:
        params = ", ".join(f"{p.name}: {p.type}" for p in rt.input_params)
        rets = ", ".join(f"{p.type}" for p in rt.output_params)
        syms[rt.name] = (f"(routine) {rt.name}({params}) -> ({rets})", rt.line, rt.column)

    return syms


def _word_at(text: str, line: int, character: int) -> str:
    """Extract the identifier word at LSP position (*line*, *character*) (0-indexed)."""
    lines = text.splitlines()
    if line >= len(lines):
        return ""
    row = lines[line]
    # Find word boundaries
    start = character
    while start > 0 and (row[start - 1].isalnum() or row[start - 1] == "_"):
        start -= 1
    end = character
    while end < len(row) and (row[end].isalnum() or row[end] == "_"):
        end += 1
    return row[start:end]


def _build_scope_map(
    program: Optional[Program],
) -> dict[tuple[int, int], tuple[str, TypeDescriptor]]:
    """Map (line, col) -> (name, type) for every declaration in the AST.

    Lines and columns are 1-indexed (as stored in AST nodes).
    """
    scope_map: dict[tuple[int, int], tuple[str, TypeDescriptor]] = {}
    if program is None:
        return scope_map

    def add(line: int, col: int, name: str, td: TypeDescriptor) -> None:
        scope_map[(line, col)] = (name, td)

    for at in program.autotuners:
        for p in at.input_params:
            add(p.line, p.column, p.name, p.type)
        for p in at.output_params:
            add(p.line, p.column, p.name, p.type)
        for stmt in at.var_decls:
            from .ast_nodes import VarDeclStmt
            if isinstance(stmt, VarDeclStmt):
                add(stmt.line, stmt.column, stmt.name, stmt.type)
        for state in at.states:
            for p in state.input_params:
                add(p.line, p.column, p.name, p.type)
            for stmt in state.body:
                from .ast_nodes import VarDeclStmt
                if isinstance(stmt, VarDeclStmt):
                    add(stmt.line, stmt.column, stmt.name, stmt.type)

    return scope_map


def _find_decl_for_word(
    word: str,
    lsp_line: int,
    program: Optional[Program],
) -> Optional[tuple[int, int]]:
    """Try to find a declaration site for *word* near LSP line *lsp_line*.

    Returns (lsp_line, lsp_col) or None.
    """
    if program is None or not word:
        return None

    ast_line = lsp_line + 1

    for at in program.autotuners:
        if at.name == word:
            return (at.line - 1, at.column - 1)
        for p in at.input_params + at.output_params:
            if p.name == word:
                return (p.line - 1, p.column - 1)
        from .ast_nodes import VarDeclStmt
        for stmt in at.var_decls:
            if isinstance(stmt, VarDeclStmt) and stmt.name == word:
                return (stmt.line - 1, stmt.column - 1)
        for state in at.states:
            if state.name == word:
                return (state.line - 1, state.column - 1)
            for p in state.input_params:
                if p.name == word:
                    return (p.line - 1, p.column - 1)
            for stmt in state.body:
                if isinstance(stmt, VarDeclStmt) and stmt.name == word:
                    return (stmt.line - 1, stmt.column - 1)

    for rt in program.routines:
        if rt.name == word:
            return (rt.line - 1, rt.column - 1)

    return None


def _hover_text_for_word(
    word: str,
    lsp_line: int,
    program: Optional[Program],
) -> Optional[str]:
    """Build a markdown hover string for *word*."""
    if not word:
        return None

    # Built-in function
    sig = DEFAULT_REGISTRY.get_function(word)
    if sig:
        return f"```\n(function) {sig.display()}\n```"

    if program is None:
        return None

    # Walk scopes in the AST
    from .ast_nodes import VarDeclStmt

    for at in program.autotuners:
        if at.name == word:
            inputs = ", ".join(f"{p.name}: {p.type}" for p in at.input_params)
            outputs = ", ".join(f"{p.type}" for p in at.output_params)
            return f"```\n(autotuner) {at.name}({inputs}) -> ({outputs})\n```"
        for p in at.input_params:
            if p.name == word:
                return f"```\n(input param) {p.name}: {p.type}\n```"
        for p in at.output_params:
            if p.name == word:
                return f"```\n(output param) {p.name}: {p.type}\n```"
        for stmt in at.var_decls:
            if isinstance(stmt, VarDeclStmt) and stmt.name == word:
                return f"```\n(variable) {stmt.name}: {stmt.type}\n```"
        for state in at.states:
            if state.name == word:
                return f"```\n(state) {state.name}\n```"
            for p in state.input_params:
                if p.name == word:
                    return f"```\n(state param) {p.name}: {p.type}\n```"
            for stmt in state.body:
                if isinstance(stmt, VarDeclStmt) and stmt.name == word:
                    return f"```\n(variable) {stmt.name}: {stmt.type}\n```"

    for rt in program.routines:
        if rt.name == word:
            inputs = ", ".join(f"{p.name}: {p.type}" for p in rt.input_params)
            outputs = ", ".join(f"{p.type}" for p in rt.output_params)
            return f"```\n(routine) {rt.name}({inputs}) -> ({outputs})\n```"

    return None


# ---------------------------------------------------------------------------
# Completion keywords / snippets
# ---------------------------------------------------------------------------

_ALL_KEYWORDS = [
    "autotuner", "routine", "state", "start", "uses", "terminal",
    "if", "elif", "else", "true", "false", "nil", "config",
    "float", "int", "bool", "string", "Quantity", "Config",
    "Connection", "Connections", "Gname", "Error",
]


def _completion_items_for(
    word_prefix: str,
    program: Optional[Program],
) -> list[lsp.CompletionItem]:
    items: list[lsp.CompletionItem] = []
    seen: set[str] = set()

    def add(label: str, kind: lsp.CompletionItemKind, detail: str = "") -> None:
        if label not in seen and label.startswith(word_prefix):
            seen.add(label)
            items.append(lsp.CompletionItem(
                label=label,
                kind=kind,
                detail=detail or None,
            ))

    # Keywords
    for kw in _ALL_KEYWORDS:
        add(kw, lsp.CompletionItemKind.Keyword)

    # Built-in functions
    for name in DEFAULT_REGISTRY.all_function_names():
        sig = DEFAULT_REGISTRY.get_function(name)
        add(name, lsp.CompletionItemKind.Function,
            sig.display() if sig else "")

    # Symbols from the program
    if program:
        from .ast_nodes import VarDeclStmt
        for at in program.autotuners:
            add(at.name, lsp.CompletionItemKind.Class)
            for p in at.input_params + at.output_params:
                add(p.name, lsp.CompletionItemKind.Variable, str(p.type))
            for stmt in at.var_decls:
                if isinstance(stmt, VarDeclStmt):
                    add(stmt.name, lsp.CompletionItemKind.Variable, str(stmt.type))
            for state in at.states:
                add(state.name, lsp.CompletionItemKind.Module)
        for rt in program.routines:
            add(rt.name, lsp.CompletionItemKind.Function)

    return items


# ---------------------------------------------------------------------------
# Server
# ---------------------------------------------------------------------------

falcon_server = LanguageServer(SERVER_NAME, SERVER_VERSION)

# uri -> _DocumentState
_docs: dict[str, _DocumentState] = {}


def _publish(server: LanguageServer, uri: str, state: _DocumentState) -> None:
    diags = [_to_lsp_diag(e) for e in state.parse_errors]
    diags += [_to_lsp_diag(d) for d in state.diagnostics]
    server.publish_diagnostics(uri, diags)


@falcon_server.feature(lsp.TEXT_DOCUMENT_DID_OPEN)
def did_open(server: LanguageServer, params: lsp.DidOpenTextDocumentParams) -> None:
    uri = params.text_document.uri
    state = _analyse(params.text_document.text)
    _docs[uri] = state
    _publish(server, uri, state)


@falcon_server.feature(lsp.TEXT_DOCUMENT_DID_CHANGE)
def did_change(server: LanguageServer, params: lsp.DidChangeTextDocumentParams) -> None:
    uri = params.text_document.uri
    # Full sync: take the last change
    text = params.content_changes[-1].text
    state = _analyse(text)
    _docs[uri] = state
    _publish(server, uri, state)


@falcon_server.feature(lsp.TEXT_DOCUMENT_DID_CLOSE)
def did_close(server: LanguageServer, params: lsp.DidCloseTextDocumentParams) -> None:
    uri = params.text_document.uri
    _docs.pop(uri, None)
    # Clear diagnostics
    server.publish_diagnostics(uri, [])


@falcon_server.feature(lsp.TEXT_DOCUMENT_HOVER)
def hover(
    server: LanguageServer, params: lsp.HoverParams
) -> Optional[lsp.Hover]:
    uri = params.text_document.uri
    state = _docs.get(uri)
    if state is None:
        return None
    pos = params.position
    word = _word_at(state.text, pos.line, pos.character)
    text = _hover_text_for_word(word, pos.line, state.program)
    if text is None:
        return None
    return lsp.Hover(
        contents=lsp.MarkupContent(kind=lsp.MarkupKind.Markdown, value=text)
    )


@falcon_server.feature(
    lsp.TEXT_DOCUMENT_COMPLETION,
    lsp.CompletionOptions(trigger_characters=[".", " "]),
)
def completion(
    server: LanguageServer, params: lsp.CompletionParams
) -> lsp.CompletionList:
    uri = params.text_document.uri
    state = _docs.get(uri)
    pos = params.position
    word_prefix = ""
    if state:
        word_prefix = _word_at(state.text, pos.line, pos.character)
    items = _completion_items_for(word_prefix, state.program if state else None)
    return lsp.CompletionList(is_incomplete=False, items=items)


@falcon_server.feature(lsp.TEXT_DOCUMENT_DEFINITION)
def definition(
    server: LanguageServer, params: lsp.DefinitionParams
) -> Optional[lsp.Location]:
    uri = params.text_document.uri
    state = _docs.get(uri)
    if state is None:
        return None
    pos = params.position
    word = _word_at(state.text, pos.line, pos.character)
    result = _find_decl_for_word(word, pos.line, state.program)
    if result is None:
        return None
    def_line, def_col = result
    return lsp.Location(
        uri=uri,
        range=lsp.Range(
            start=lsp.Position(line=def_line, character=def_col),
            end=lsp.Position(line=def_line, character=def_col + len(word)),
        ),
    )


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    logging.basicConfig(level=logging.WARNING)
    falcon_server.start_io()


if __name__ == "__main__":
    main()
