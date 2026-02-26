"""Tests for the Falcon DSL lexer."""

import pytest
from falcon_lsp.lexer import Token, tokenize


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def tok_types(text: str) -> list[str]:
    return [t.type for t in tokenize(text) if t.type != "EOF"]


def tok_values(text: str) -> list[str]:
    return [t.value for t in tokenize(text) if t.type != "EOF"]


# ---------------------------------------------------------------------------
# Keywords
# ---------------------------------------------------------------------------

class TestKeywords:
    def test_autotuner(self):
        assert tok_types("autotuner") == ["AUTOTUNER"]

    def test_routine(self):
        assert tok_types("routine") == ["ROUTINE"]

    def test_state(self):
        assert tok_types("state") == ["STATE"]

    def test_start(self):
        assert tok_types("start") == ["START"]

    def test_uses(self):
        assert tok_types("uses") == ["USES"]

    def test_terminal(self):
        assert tok_types("terminal") == ["TERMINAL"]

    def test_if_elif_else(self):
        assert tok_types("if elif else") == ["IF", "ELIF", "ELSE"]

    def test_true_false(self):
        assert tok_types("true false") == ["TRUE", "FALSE"]

    def test_nil(self):
        assert tok_types("nil") == ["NIL"]

    def test_config_var(self):
        assert tok_types("config") == ["CONFIG_VAR"]

    def test_type_keywords(self):
        kws = "float int bool string Quantity Config Connection Connections Gname Error"
        expected = [
            "FLOAT_KW", "INT_KW", "BOOL_KW", "STRING_KW",
            "QUANTITY_KW", "CONFIG_KW", "CONNECTION_KW",
            "CONNECTIONS_KW", "GNAME_KW", "ERROR_KW",
        ]
        assert tok_types(kws) == expected


# ---------------------------------------------------------------------------
# Identifiers
# ---------------------------------------------------------------------------

class TestIdentifiers:
    def test_simple(self):
        toks = tokenize("myVar")
        assert toks[0].type == "IDENTIFIER"
        assert toks[0].value == "myVar"

    def test_underscore_start(self):
        assert tok_types("_foo") == ["IDENTIFIER"]

    def test_mixed_case(self):
        assert tok_types("CamelCase") == ["IDENTIFIER"]

    def test_with_digits(self):
        assert tok_types("var123") == ["IDENTIFIER"]

    def test_does_not_match_keyword_prefix(self):
        # "autotunerX" is an identifier, not a keyword
        toks = tokenize("autotunerX")
        assert toks[0].type == "IDENTIFIER"


# ---------------------------------------------------------------------------
# Literals
# ---------------------------------------------------------------------------

class TestLiterals:
    def test_integer(self):
        toks = tokenize("42")
        assert toks[0].type == "INTEGER"
        assert toks[0].value == "42"

    def test_float(self):
        toks = tokenize("3.14")
        assert toks[0].type == "DOUBLE"
        assert toks[0].value == "3.14"

    def test_string(self):
        toks = tokenize('"hello world"')
        assert toks[0].type == "STRING"
        assert toks[0].value == "hello world"  # quotes stripped

    def test_string_empty(self):
        toks = tokenize('""')
        assert toks[0].type == "STRING"
        assert toks[0].value == ""

    def test_zero(self):
        toks = tokenize("0")
        assert toks[0].type == "INTEGER"

    def test_float_zero(self):
        toks = tokenize("0.0")
        assert toks[0].type == "DOUBLE"


# ---------------------------------------------------------------------------
# Operators
# ---------------------------------------------------------------------------

class TestOperators:
    def test_arrow(self):
        assert tok_types("->") == ["ARROW"]

    def test_comparison_ops(self):
        assert tok_types("== != < > <= >=") == ["EQ", "NE", "LL", "GG", "LE", "GE"]

    def test_logical_ops(self):
        assert tok_types("&& ||") == ["AND", "OR"]

    def test_not(self):
        assert tok_types("!") == ["NOT"]

    def test_arithmetic(self):
        assert tok_types("+ - * /") == ["PLUS", "MINUS", "MUL", "DIV"]

    def test_assign(self):
        assert tok_types("=") == ["ASSIGN"]

    def test_assign_vs_eq(self):
        types = tok_types("= ==")
        assert types == ["ASSIGN", "EQ"]

    def test_brackets(self):
        assert tok_types("[ ]") == ["LBRACKET", "RBRACKET"]

    def test_braces(self):
        assert tok_types("{ }") == ["LBRACE", "RBRACE"]

    def test_parens(self):
        assert tok_types("( )") == ["LPAREN", "RPAREN"]

    def test_comma_semicolon_dot(self):
        assert tok_types(", ; .") == ["COMMA", "SEMICOLON", "DOT"]


# ---------------------------------------------------------------------------
# Comments
# ---------------------------------------------------------------------------

class TestComments:
    def test_line_comment_skipped(self):
        assert tok_types("// this is a comment") == []

    def test_comment_after_code(self):
        types = tok_types("int x; // declare x")
        assert types == ["INT_KW", "IDENTIFIER", "SEMICOLON"]

    def test_comment_does_not_eat_next_line(self):
        code = "// first\nint x;"
        types = tok_types(code)
        assert types == ["INT_KW", "IDENTIFIER", "SEMICOLON"]


# ---------------------------------------------------------------------------
# Whitespace
# ---------------------------------------------------------------------------

class TestWhitespace:
    def test_spaces_skipped(self):
        assert tok_types("  int  x  ") == ["INT_KW", "IDENTIFIER"]

    def test_tabs_skipped(self):
        assert tok_types("\tint\tx") == ["INT_KW", "IDENTIFIER"]


# ---------------------------------------------------------------------------
# Source locations
# ---------------------------------------------------------------------------

class TestLocations:
    def test_line_numbers(self):
        toks = tokenize("int\nbool")
        assert toks[0].line == 1
        assert toks[1].line == 2

    def test_column_numbers(self):
        toks = tokenize("int x")
        assert toks[0].column == 1
        assert toks[1].column == 5

    def test_eof_token(self):
        toks = tokenize("int")
        assert toks[-1].type == "EOF"


# ---------------------------------------------------------------------------
# Multi-line code
# ---------------------------------------------------------------------------

class TestMultiLine:
    def test_autotuner_header(self):
        code = "autotuner Foo (int a) -> (bool b)"
        types = tok_types(code)
        assert types == [
            "AUTOTUNER", "IDENTIFIER",
            "LPAREN", "INT_KW", "IDENTIFIER", "RPAREN",
            "ARROW",
            "LPAREN", "BOOL_KW", "IDENTIFIER", "RPAREN",
        ]

    def test_full_simple_autotuner(self):
        code = """
autotuner Calc (int a) -> (int r) {
  r = 0;
  start -> done;
  state done { terminal; }
}
"""
        types = tok_types(code)
        # Just ensure no crash and key tokens present
        assert "AUTOTUNER" in types
        assert "STATE" in types
        assert "TERMINAL" in types
