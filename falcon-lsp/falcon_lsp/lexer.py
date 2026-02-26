"""Tokenizer for the Falcon autotuner DSL, mirroring lexer.l."""

from __future__ import annotations

import re
from dataclasses import dataclass
from typing import Iterator


# ---------------------------------------------------------------------------
# Token types
# ---------------------------------------------------------------------------

# Keywords
KW_TYPES = {
    "autotuner": "AUTOTUNER",
    "routine": "ROUTINE",
    "state": "STATE",
    "start": "START",
    "uses": "USES",
    "terminal": "TERMINAL",
    "if": "IF",
    "elif": "ELIF",
    "else": "ELSE",
    "true": "TRUE",
    "false": "FALSE",
    "nil": "NIL",
    "config": "CONFIG_VAR",
    # Type keywords
    "float": "FLOAT_KW",
    "int": "INT_KW",
    "bool": "BOOL_KW",
    "string": "STRING_KW",
    "Quantity": "QUANTITY_KW",
    "Config": "CONFIG_KW",
    "Connection": "CONNECTION_KW",
    "Connections": "CONNECTIONS_KW",
    "Gname": "GNAME_KW",
    "Error": "ERROR_KW",
}

# Ordered longest-first to avoid partial matches
_OPERATORS = [
    ("->", "ARROW"),
    ("==", "EQ"),
    ("!=", "NE"),
    ("<=", "LE"),
    (">=", "GE"),
    ("&&", "AND"),
    ("||", "OR"),
    ("<", "LL"),
    (">", "GG"),
    ("[", "LBRACKET"),
    ("]", "RBRACKET"),
    ("{", "LBRACE"),
    ("}", "RBRACE"),
    ("(", "LPAREN"),
    (")", "RPAREN"),
    ("=", "ASSIGN"),
    (",", "COMMA"),
    (";", "SEMICOLON"),
    (".", "DOT"),
    ("+", "PLUS"),
    ("-", "MINUS"),
    ("*", "MUL"),
    ("/", "DIV"),
    ("!", "NOT"),
]

# Build a single regex from all token patterns (order matters!)
_TOKEN_PATTERNS: list[tuple[str, str]] = [
    ("COMMENT", r"//[^\n]*"),
    ("DOUBLE", r"[0-9]+\.[0-9]+"),
    ("INTEGER", r"[0-9]+"),
    ("STRING", r'"[^"]*"'),
    ("IDENTIFIER", r"[a-zA-Z_][a-zA-Z0-9_]*"),
    ("ARROW", r"->"),
    ("EQ", r"=="),
    ("NE", r"!="),
    ("LE", r"<="),
    ("GE", r">="),
    ("AND", r"&&"),
    ("OR", r"\|\|"),
    ("LL", r"<"),
    ("GG", r">"),
    ("LBRACKET", r"\["),
    ("RBRACKET", r"\]"),
    ("LBRACE", r"\{"),
    ("RBRACE", r"\}"),
    ("LPAREN", r"\("),
    ("RPAREN", r"\)"),
    ("ASSIGN", r"="),
    ("COMMA", r","),
    ("SEMICOLON", r";"),
    ("DOT", r"\."),
    ("PLUS", r"\+"),
    ("MINUS", r"-"),
    ("MUL", r"\*"),
    ("DIV", r"/"),
    ("NOT", r"!"),
    ("NEWLINE", r"\n"),
    ("WHITESPACE", r"[ \t\r]+"),
    ("UNKNOWN", r"."),
]

_MASTER_PATTERN = re.compile(
    "|".join(f"(?P<{name}>{pat})" for name, pat in _TOKEN_PATTERNS)
)


@dataclass
class Token:
    """A single lexical token."""

    type: str
    value: str
    line: int
    column: int

    def __repr__(self) -> str:
        return f"Token({self.type!r}, {self.value!r}, {self.line}:{self.column})"


class LexError(Exception):
    """Raised for unrecognised characters."""

    def __init__(self, message: str, line: int, column: int) -> None:
        super().__init__(message)
        self.line = line
        self.column = column


def tokenize(text: str) -> list[Token]:
    """Tokenize *text* and return a list of :class:`Token` objects.

    The final token is always ``Token('EOF', '', line, col)``.
    Unknown characters are skipped with a warning rather than raising.
    """
    tokens: list[Token] = []
    line = 1
    line_start = 0

    for mo in _MASTER_PATTERN.finditer(text):
        kind = mo.lastgroup
        value = mo.group()
        col = mo.start() - line_start + 1

        if kind == "NEWLINE":
            line += 1
            line_start = mo.end()
            continue
        if kind in ("WHITESPACE", "COMMENT"):
            # Track newlines inside comments (though comments are // style)
            continue
        if kind == "UNKNOWN":
            # Skip unknown characters silently; the parser will surface errors
            continue

        if kind == "IDENTIFIER":
            kind = KW_TYPES.get(value, "IDENTIFIER")
        elif kind == "STRING":
            # Strip surrounding quotes
            value = value[1:-1]

        tokens.append(Token(kind, value, line, col))

    # Determine EOF position
    last_line = line
    last_col = len(text) - line_start + 1 if text else 1
    tokens.append(Token("EOF", "", last_line, last_col))
    return tokens


def tokenize_iter(text: str) -> Iterator[Token]:
    """Yield tokens one at a time (including the final EOF)."""
    yield from tokenize(text)
