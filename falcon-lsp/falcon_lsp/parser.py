"""Recursive-descent parser for the Falcon autotuner DSL.

Mirrors the grammar defined in parser.y.  The parser is intentionally
fault-tolerant: it collects errors and tries to continue rather than
aborting on the first problem.
"""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Optional

from .ast_nodes import (
    AssignStmt,
    AutotunerDecl,
    BinaryExpr,
    CallArg,
    CallExpr,
    Expr,
    ExprStmt,
    IfStmt,
    IndexExpr,
    LiteralExpr,
    MemberExpr,
    MethodCallExpr,
    NilLiteralExpr,
    ParamDecl,
    Program,
    RoutineDecl,
    StateDecl,
    Stmt,
    TerminalStmt,
    TransitionStmt,
    TypeDescriptor,
    UnaryExpr,
    VarDeclStmt,
    VarExpr,
)
from .lexer import Token, tokenize


@dataclass
class ParseError:
    """A single parse error with source location."""

    line: int
    col: int
    message: str

    def __str__(self) -> str:
        return f"{self.line}:{self.col}: {self.message}"


# Operator precedence (higher = tighter binding)
_PREC: dict[str, int] = {
    "OR": 1,
    "AND": 2,
    "EQ": 3,
    "NE": 3,
    "LL": 4,
    "GG": 4,
    "LE": 4,
    "GE": 4,
    "PLUS": 5,
    "MINUS": 5,
    "MUL": 6,
    "DIV": 6,
}

_OP_STR: dict[str, str] = {
    "OR": "||",
    "AND": "&&",
    "EQ": "==",
    "NE": "!=",
    "LL": "<",
    "GG": ">",
    "LE": "<=",
    "GE": ">=",
    "PLUS": "+",
    "MINUS": "-",
    "MUL": "*",
    "DIV": "/",
}

# Token types that start a type specifier
_TYPE_KWS = frozenset({
    "INT_KW", "FLOAT_KW", "BOOL_KW", "STRING_KW",
    "QUANTITY_KW", "CONFIG_KW", "CONNECTION_KW",
    "CONNECTIONS_KW", "GNAME_KW", "ERROR_KW",
})

_TYPE_NAMES = {
    "INT_KW": "int",
    "FLOAT_KW": "float",
    "BOOL_KW": "bool",
    "STRING_KW": "string",
    "QUANTITY_KW": "Quantity",
    "CONFIG_KW": "Config",
    "CONNECTION_KW": "Connection",
    "CONNECTIONS_KW": "Connections",
    "GNAME_KW": "Gname",
    "ERROR_KW": "Error",
}


class Parser:
    """Recursive-descent parser producing a :class:`~falcon_lsp.ast_nodes.Program`."""

    def __init__(self, tokens: list[Token]) -> None:
        self._tokens = tokens
        self._pos = 0
        self._errors: list[ParseError] = []

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def parse(self) -> tuple[Optional[Program], list[ParseError]]:
        """Parse the token stream and return ``(program, errors)``."""
        program = self._parse_program()
        return program, self._errors

    # ------------------------------------------------------------------
    # Token stream helpers
    # ------------------------------------------------------------------

    def _peek(self, offset: int = 0) -> Token:
        idx = self._pos + offset
        if idx < len(self._tokens):
            return self._tokens[idx]
        return self._tokens[-1]  # EOF

    def _advance(self) -> Token:
        tok = self._tokens[self._pos]
        if self._pos < len(self._tokens) - 1:
            self._pos += 1
        return tok

    def _check(self, *types: str) -> bool:
        return self._peek().type in types

    def _match(self, *types: str) -> Optional[Token]:
        """Consume and return the next token if its type is in *types*."""
        if self._peek().type in types:
            return self._advance()
        return None

    def _expect(self, *types: str) -> Optional[Token]:
        """Consume a token of one of *types*, recording an error if missing."""
        tok = self._match(*types)
        if tok is None:
            cur = self._peek()
            self._error(
                cur.line, cur.column,
                f"expected {' or '.join(types)}, got {cur.type!r} ({cur.value!r})",
            )
        return tok

    def _error(self, line: int, col: int, msg: str) -> None:
        self._errors.append(ParseError(line, col, msg))

    # Synchronisation sets for panic-mode recovery
    _STMT_FOLLOW = frozenset({
        "STATE", "RBRACE", "EOF",
        "AUTOTUNER", "ROUTINE",
    })

    def _sync_to(self, *stop_types: str) -> None:
        """Skip tokens until one of *stop_types* (or EOF) is seen."""
        while not self._check(*stop_types, "EOF"):
            self._advance()

    # ------------------------------------------------------------------
    # Grammar rules
    # ------------------------------------------------------------------

    def _parse_program(self) -> Program:
        prog = Program()
        while not self._check("EOF"):
            if self._check("AUTOTUNER"):
                at = self._parse_autotuner()
                if at:
                    prog.autotuners.append(at)
            elif self._check("ROUTINE"):
                rt = self._parse_routine()
                if rt:
                    prog.routines.append(rt)
            else:
                cur = self._peek()
                self._error(cur.line, cur.column, f"unexpected token {cur.type!r} at top level")
                self._advance()
        return prog

    # ------------------------------------------------------------------ autotuner

    def _parse_autotuner(self) -> Optional[AutotunerDecl]:
        tok = self._expect("AUTOTUNER")
        if tok is None:
            return None
        name_tok = self._expect("IDENTIFIER")
        if name_tok is None:
            self._sync_to("LBRACE", "AUTOTUNER", "ROUTINE", "EOF")
            return None
        name = name_tok.value
        at = AutotunerDecl(name=name, line=tok.line, column=tok.column)

        at.input_params = self._parse_input_params()
        if not self._check("ARROW"):
            cur = self._peek()
            self._error(cur.line, cur.column, f"expected '->' after autotuner inputs, got {cur.type!r}")
        else:
            self._advance()
        at.output_params = self._parse_output_params()

        if not self._expect("LBRACE"):
            self._sync_to("AUTOTUNER", "ROUTINE", "EOF")
            return at

        # Body
        at.uses = self._parse_uses_clause()
        at.var_decls = self._parse_autotuner_var_decls()
        entry, entry_params = self._parse_entry_state()
        at.entry_state = entry
        at.entry_params = entry_params
        at.states = self._parse_state_list()

        self._expect("RBRACE")
        return at

    def _parse_input_params(self) -> list[ParamDecl]:
        """Parse optional ``(param_list)``."""
        if not self._check("LPAREN"):
            return []
        self._advance()
        params = self._parse_param_decl_list()
        self._expect("RPAREN")
        return params

    def _parse_output_params(self) -> list[ParamDecl]:
        """Parse optional ``(param_list)``."""
        if not self._check("LPAREN"):
            return []
        self._advance()
        params = self._parse_param_decl_list()
        self._expect("RPAREN")
        return params

    def _parse_param_decl_list(self) -> list[ParamDecl]:
        params: list[ParamDecl] = []
        if self._check("RPAREN"):
            return params
        p = self._parse_param_decl()
        if p:
            params.append(p)
        while self._match("COMMA"):
            if self._check("RPAREN"):
                break
            p = self._parse_param_decl()
            if p:
                params.append(p)
        return params

    def _parse_param_decl(self) -> Optional[ParamDecl]:
        type_tok = self._peek()
        td = self._parse_type_spec()
        if td is None:
            return None
        name_tok = self._expect("IDENTIFIER")
        if name_tok is None:
            return None
        default = None
        if self._match("ASSIGN"):
            default = self._parse_expr()
        return ParamDecl(type=td, name=name_tok.value, default_value=default,
                         line=type_tok.line, column=type_tok.column)

    def _parse_type_spec(self) -> Optional[TypeDescriptor]:
        tok = self._peek()
        if tok.type in _TYPE_KWS:
            self._advance()
            return TypeDescriptor(base_type=_TYPE_NAMES[tok.type])
        self._error(tok.line, tok.column, f"expected type specifier, got {tok.type!r}")
        return None

    def _parse_uses_clause(self) -> list[str]:
        if not self._check("USES"):
            return []
        self._advance()
        names: list[str] = []
        if self._check("IDENTIFIER"):
            names.append(self._advance().value)
        while self._match("COMMA"):
            if self._check("IDENTIFIER"):
                names.append(self._advance().value)
        self._expect("SEMICOLON")
        return names

    def _parse_autotuner_var_decls(self) -> list[Stmt]:
        """Parse var-decl / assign / expr statements before the entry state."""
        stmts: list[Stmt] = []
        while not self._check("START", "STATE", "RBRACE", "EOF"):
            s = self._parse_stmt(in_state=False)
            if s:
                stmts.append(s)
            else:
                # Recovery: skip one token to avoid infinite loop
                if not self._check("START", "STATE", "RBRACE", "EOF"):
                    self._advance()
        return stmts

    def _parse_entry_state(self) -> tuple[str, list]:
        name = ""
        params: list = []
        if not self._check("START"):
            cur = self._peek()
            self._error(cur.line, cur.column, f"expected 'start', got {cur.type!r}")
            return name, params
        self._advance()  # consume START
        self._expect("ARROW")
        name_tok = self._expect("IDENTIFIER")
        if name_tok:
            name = name_tok.value
        if self._check("LPAREN"):
            self._advance()
            params = self._parse_expr_list()
            self._expect("RPAREN")
        self._expect("SEMICOLON")
        return name, params

    def _parse_state_list(self) -> list[StateDecl]:
        states: list[StateDecl] = []
        while self._check("STATE"):
            s = self._parse_state()
            if s:
                states.append(s)
        return states

    def _parse_state(self) -> Optional[StateDecl]:
        tok = self._expect("STATE")
        if tok is None:
            return None
        name_tok = self._expect("IDENTIFIER")
        if name_tok is None:
            self._sync_to("LBRACE", "STATE", "RBRACE", "EOF")
            return None
        name = name_tok.value
        state = StateDecl(name=name, line=tok.line, column=tok.column)

        # Optional input params
        if self._check("LPAREN"):
            self._advance()
            state.input_params = self._parse_param_decl_list()
            self._expect("RPAREN")

        if not self._expect("LBRACE"):
            self._sync_to("STATE", "RBRACE", "EOF")
            return state
        state.body = self._parse_stmt_list()
        self._expect("RBRACE")
        return state

    # ------------------------------------------------------------------ routine

    def _parse_routine(self) -> Optional[RoutineDecl]:
        tok = self._expect("ROUTINE")
        if tok is None:
            return None
        name_tok = self._expect("IDENTIFIER")
        if name_tok is None:
            return None
        rt = RoutineDecl(name=name_tok.value, line=tok.line, column=tok.column)
        rt.input_params = self._parse_input_params()
        if self._match("ARROW"):
            rt.output_params = self._parse_output_params()
        return rt

    # ------------------------------------------------------------------ statements

    def _parse_stmt_list(self) -> list[Stmt]:
        stmts: list[Stmt] = []
        while not self._check("RBRACE", "EOF"):
            s = self._parse_stmt(in_state=True)
            if s:
                stmts.append(s)
            else:
                if not self._check("RBRACE", "EOF"):
                    self._advance()
        return stmts

    def _parse_stmt(self, in_state: bool = True) -> Optional[Stmt]:
        tok = self._peek()

        # terminal
        if self._check("TERMINAL"):
            self._advance()
            self._expect("SEMICOLON")
            return TerminalStmt(line=tok.line, column=tok.column)

        # transition -> state_name
        if self._check("ARROW"):
            self._advance()
            target_tok = self._expect("IDENTIFIER")
            target = target_tok.value if target_tok else ""
            params: list[Expr] = []
            if self._check("LPAREN"):
                self._advance()
                params = self._parse_expr_list()
                self._expect("RPAREN")
            self._expect("SEMICOLON")
            return TransitionStmt(target=target, params=params,
                                  line=tok.line, column=tok.column)

        # if statement
        if self._check("IF"):
            return self._parse_if_stmt()

        # var decl: starts with a type keyword
        if tok.type in _TYPE_KWS:
            return self._parse_var_decl_stmt()

        # multi-assign or expr-stmt: starts with IDENTIFIER
        if tok.type == "IDENTIFIER":
            # Peek ahead to see if this is an assignment:
            #   "a = ..." or "a, b = ..."
            return self._parse_assign_or_expr_stmt()

        # CONFIG_VAR as expression statement
        if tok.type == "CONFIG_VAR":
            expr = self._parse_expr()
            self._expect("SEMICOLON")
            if expr:
                return ExprStmt(expr=expr, line=tok.line, column=tok.column)
            return None

        # Anything else: try to parse as an expression statement
        expr = self._parse_expr()
        if expr:
            self._expect("SEMICOLON")
            return ExprStmt(expr=expr, line=tok.line, column=tok.column)

        cur = self._peek()
        self._error(cur.line, cur.column, f"unexpected token in statement: {cur.type!r}")
        return None

    def _parse_var_decl_stmt(self) -> Optional[VarDeclStmt]:
        tok = self._peek()
        td = self._parse_type_spec()
        if td is None:
            return None
        name_tok = self._expect("IDENTIFIER")
        if name_tok is None:
            self._sync_to("SEMICOLON", "RBRACE", "EOF")
            self._match("SEMICOLON")
            return None
        init: Optional[Expr] = None
        if self._match("ASSIGN"):
            init = self._parse_expr()
        self._expect("SEMICOLON")
        return VarDeclStmt(type=td, name=name_tok.value, initializer=init,
                           line=tok.line, column=tok.column)

    def _parse_assign_or_expr_stmt(self) -> Optional[Stmt]:
        """Attempt multi-assign (``a, b = expr``) or fall back to expr-stmt."""
        start_tok = self._peek()

        # Collect identifier list
        targets: list[str] = []
        if self._check("IDENTIFIER"):
            targets.append(self._peek().value)
            saved_pos = self._pos
            self._advance()

            while self._check("COMMA"):
                comma_pos = self._pos
                self._advance()
                if self._check("IDENTIFIER"):
                    targets.append(self._peek().value)
                    self._advance()
                else:
                    # Not a multi-assign – roll back the comma
                    self._pos = comma_pos
                    break

            if self._check("ASSIGN"):
                self._advance()  # consume '='
                val = self._parse_expr()
                self._expect("SEMICOLON")
                return AssignStmt(targets=targets, value=val,
                                  line=start_tok.line, column=start_tok.column)

            # Not an assignment – restore and parse as expression
            self._pos = saved_pos

        expr = self._parse_expr()
        if expr:
            self._expect("SEMICOLON")
            return ExprStmt(expr=expr, line=start_tok.line, column=start_tok.column)
        return None

    def _parse_if_stmt(self) -> Optional[IfStmt]:
        tok = self._expect("IF")
        if tok is None:
            return None
        self._expect("LPAREN")
        cond = self._parse_expr()
        self._expect("RPAREN")
        self._expect("LBRACE")
        then_body = self._parse_stmt_list()
        self._expect("RBRACE")
        else_body = self._parse_elif_chain()
        return IfStmt(condition=cond, then_body=then_body, else_body=else_body,
                      line=tok.line, column=tok.column)

    def _parse_elif_chain(self) -> list[Stmt]:
        if self._check("ELIF"):
            tok = self._advance()
            self._expect("LPAREN")
            cond = self._parse_expr()
            self._expect("RPAREN")
            self._expect("LBRACE")
            then_body = self._parse_stmt_list()
            self._expect("RBRACE")
            else_body = self._parse_elif_chain()
            return [IfStmt(condition=cond, then_body=then_body, else_body=else_body,
                           line=tok.line, column=tok.column)]
        if self._check("ELSE"):
            self._advance()
            self._expect("LBRACE")
            body = self._parse_stmt_list()
            self._expect("RBRACE")
            return body
        return []

    # ------------------------------------------------------------------ expressions

    def _parse_expr(self, min_prec: int = 0) -> Optional[Expr]:
        """Parse an expression using precedence climbing."""
        left = self._parse_unary()
        if left is None:
            return None

        while True:
            tok = self._peek()
            prec = _PREC.get(tok.type, -1)
            if prec < min_prec or prec < 0:
                break
            op_tok = self._advance()
            right = self._parse_unary()
            if right is None:
                break
            # Keep climbing for left-associative operators
            while True:
                next_tok = self._peek()
                next_prec = _PREC.get(next_tok.type, -1)
                if next_prec <= prec or next_prec < 0:
                    break
                right = self._parse_binary_rhs(right, next_prec)
                if right is None:
                    break
            op_str = _OP_STR[op_tok.type]
            left = BinaryExpr(op=op_str, left=left, right=right,
                              line=op_tok.line, column=op_tok.column)
        return left

    def _parse_binary_rhs(self, left: Expr, min_prec: int) -> Optional[Expr]:
        tok = self._peek()
        prec = _PREC.get(tok.type, -1)
        if prec < min_prec:
            return left
        op_tok = self._advance()
        right = self._parse_unary()
        if right is None:
            return left
        op_str = _OP_STR[op_tok.type]
        return BinaryExpr(op=op_str, left=left, right=right,
                          line=op_tok.line, column=op_tok.column)

    def _parse_unary(self) -> Optional[Expr]:
        tok = self._peek()
        if tok.type == "NOT":
            self._advance()
            operand = self._parse_unary()
            return UnaryExpr(op="!", operand=operand, line=tok.line, column=tok.column)
        if tok.type == "MINUS":
            self._advance()
            operand = self._parse_unary()
            return UnaryExpr(op="-", operand=operand, line=tok.line, column=tok.column)
        return self._parse_postfix()

    def _parse_postfix(self) -> Optional[Expr]:
        base = self._parse_primary()
        if base is None:
            return None

        while True:
            tok = self._peek()
            if tok.type == "DOT":
                self._advance()
                member_tok = self._expect("IDENTIFIER")
                if member_tok is None:
                    break
                # Could be method call
                if self._check("LPAREN"):
                    self._advance()
                    args: list[Expr] = []
                    if not self._check("RPAREN"):
                        args = self._parse_expr_list()
                    self._expect("RPAREN")
                    base = MethodCallExpr(obj=base, method=member_tok.value, args=args,
                                         line=tok.line, column=tok.column)
                else:
                    base = MemberExpr(obj=base, member=member_tok.value,
                                      line=tok.line, column=tok.column)
            elif tok.type == "LBRACKET":
                self._advance()
                idx = self._parse_expr()
                self._expect("RBRACKET")
                base = IndexExpr(obj=base, index=idx, line=tok.line, column=tok.column)
            else:
                break
        return base

    def _parse_primary(self) -> Optional[Expr]:
        tok = self._peek()

        if tok.type == "INTEGER":
            self._advance()
            return LiteralExpr(value=int(tok.value), line=tok.line, column=tok.column)

        if tok.type == "DOUBLE":
            self._advance()
            return LiteralExpr(value=float(tok.value), line=tok.line, column=tok.column)

        if tok.type == "STRING":
            self._advance()
            return LiteralExpr(value=tok.value, line=tok.line, column=tok.column)

        if tok.type == "TRUE":
            self._advance()
            return LiteralExpr(value=True, line=tok.line, column=tok.column)

        if tok.type == "FALSE":
            self._advance()
            return LiteralExpr(value=False, line=tok.line, column=tok.column)

        if tok.type == "NIL":
            self._advance()
            return NilLiteralExpr(line=tok.line, column=tok.column)

        if tok.type == "CONFIG_VAR":
            self._advance()
            return VarExpr(name="config", line=tok.line, column=tok.column)

        if tok.type == "IDENTIFIER":
            self._advance()
            # Check for function call
            if self._check("LPAREN"):
                self._advance()
                args: list[CallArg] = []
                if not self._check("RPAREN"):
                    args = self._parse_call_arg_list()
                self._expect("RPAREN")
                return CallExpr(name=tok.value, args=args,
                                line=tok.line, column=tok.column)
            return VarExpr(name=tok.value, line=tok.line, column=tok.column)

        if tok.type == "LPAREN":
            self._advance()
            inner = self._parse_expr()
            self._expect("RPAREN")
            return inner

        cur = self._peek()
        self._error(cur.line, cur.column,
                    f"unexpected token in expression: {cur.type!r} ({cur.value!r})")
        return None

    def _parse_expr_list(self) -> list[Expr]:
        exprs: list[Expr] = []
        e = self._parse_expr()
        if e:
            exprs.append(e)
        while self._check("COMMA"):
            self._advance()
            e = self._parse_expr()
            if e:
                exprs.append(e)
        return exprs

    def _parse_call_arg_list(self) -> list[CallArg]:
        args: list[CallArg] = []
        a = self._parse_call_arg()
        if a:
            args.append(a)
        while self._check("COMMA"):
            self._advance()
            if self._check("RPAREN"):
                break
            a = self._parse_call_arg()
            if a:
                args.append(a)
        return args

    def _parse_call_arg(self) -> Optional[CallArg]:
        # Named arg: IDENTIFIER ASSIGN expr
        if self._check("IDENTIFIER") and self._peek(1).type == "ASSIGN":
            name_tok = self._advance()
            self._advance()  # consume '='
            val = self._parse_expr()
            return CallArg(value=val, keyword=name_tok.value)
        val = self._parse_expr()
        if val is None:
            return None
        return CallArg(value=val, keyword=None)


def parse(text: str) -> tuple[Optional[Program], list[ParseError]]:
    """Convenience function: tokenize *text* and parse it."""
    tokens = tokenize(text)
    return Parser(tokens).parse()
