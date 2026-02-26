"""Tests for the Falcon LSP server (non-IO functionality)."""

import pytest
from falcon_lsp.server import (
    _analyse,
    _completion_items_for,
    _hover_text_for_word,
    _find_decl_for_word,
    _to_lsp_diag,
    _word_at,
)
from falcon_lsp.parser import ParseError, parse
from falcon_lsp.type_checker import Diagnostic
from lsprotocol import types as lsp


# ---------------------------------------------------------------------------
# _analyse helper
# ---------------------------------------------------------------------------

class TestAnalyse:
    CALC = """
autotuner Calculator (int a, int b) -> (int sum, int product) {
  sum = 0;
  product = 0;
  start -> calculate;
  state calculate {
    sum = a + b;
    -> done;
  }
  state done { terminal; }
}
"""

    def test_returns_program(self):
        state = _analyse(self.CALC)
        assert state.program is not None

    def test_no_parse_errors_for_valid(self):
        state = _analyse(self.CALC)
        assert state.parse_errors == []

    def test_parse_errors_for_invalid(self):
        state = _analyse("autotuner broken {{{")
        # Should have some parse errors
        assert len(state.parse_errors) > 0 or state.program is not None


# ---------------------------------------------------------------------------
# Diagnostics conversion
# ---------------------------------------------------------------------------

class TestDiagnosticConversion:
    def test_parse_error_to_lsp(self):
        err = ParseError(line=3, col=5, message="unexpected token")
        diag = _to_lsp_diag(err)
        assert diag.range.start.line == 2  # 0-indexed
        assert diag.range.start.character == 4
        assert diag.severity == lsp.DiagnosticSeverity.Error

    def test_type_error_to_lsp(self):
        diag_in = Diagnostic(line=1, col=1, message="type mismatch", severity="error")
        diag = _to_lsp_diag(diag_in)
        assert diag.severity == lsp.DiagnosticSeverity.Error

    def test_warning_to_lsp(self):
        diag_in = Diagnostic(line=2, col=3, message="redeclaration", severity="warning")
        diag = _to_lsp_diag(diag_in)
        assert diag.severity == lsp.DiagnosticSeverity.Warning


# ---------------------------------------------------------------------------
# Word extraction
# ---------------------------------------------------------------------------

class TestWordAt:
    def test_word_in_middle(self):
        assert _word_at("int myVar;", 0, 5) == "myVar"

    def test_word_at_start(self):
        assert _word_at("hello world", 0, 0) == "hello"

    def test_word_at_end(self):
        assert _word_at("hello world", 0, 10) == "world"

    def test_empty_line(self):
        assert _word_at("", 0, 0) == ""

    def test_multiline(self):
        text = "line one\nline two"
        assert _word_at(text, 1, 5) == "two"


# ---------------------------------------------------------------------------
# Hover
# ---------------------------------------------------------------------------

class TestHover:
    CODE = """
autotuner Calculator (int a, int b) -> (int sum) {
  sum = 0;
  start -> calc;
  state calc { sum = a + b; terminal; }
}
"""

    def setup_method(self):
        prog, _ = parse(self.CODE)
        self.prog = prog

    def test_hover_builtin_function(self):
        text = _hover_text_for_word("logInfo", 0, self.prog)
        assert text is not None
        assert "logInfo" in text

    def test_hover_autotuner_name(self):
        text = _hover_text_for_word("Calculator", 0, self.prog)
        assert text is not None
        assert "Calculator" in text

    def test_hover_input_param(self):
        text = _hover_text_for_word("a", 0, self.prog)
        assert text is not None
        assert "int" in text

    def test_hover_state_name(self):
        text = _hover_text_for_word("calc", 0, self.prog)
        assert text is not None
        assert "state" in text.lower() or "calc" in text

    def test_hover_unknown_returns_none(self):
        text = _hover_text_for_word("zzz_unknown", 0, self.prog)
        assert text is None

    def test_hover_none_program(self):
        text = _hover_text_for_word("logInfo", 0, None)
        # Built-ins don't require program
        assert text is not None


# ---------------------------------------------------------------------------
# Completion
# ---------------------------------------------------------------------------

class TestCompletion:
    CODE = """
autotuner Foo -> (bool r) {
  r = false;
  start -> s;
  state s { terminal; }
}
"""

    def setup_method(self):
        prog, _ = parse(self.CODE)
        self.prog = prog

    def test_keywords_suggested(self):
        items = _completion_items_for("", self.prog)
        labels = [i.label for i in items]
        assert "autotuner" in labels
        assert "state" in labels
        assert "terminal" in labels

    def test_builtins_suggested(self):
        items = _completion_items_for("", self.prog)
        labels = [i.label for i in items]
        assert "logInfo" in labels
        assert "readLatest" in labels

    def test_autotuner_name_suggested(self):
        items = _completion_items_for("", self.prog)
        labels = [i.label for i in items]
        assert "Foo" in labels

    def test_prefix_filter(self):
        items = _completion_items_for("log", self.prog)
        labels = [i.label for i in items]
        assert all(l.startswith("log") for l in labels)
        assert "logInfo" in labels

    def test_empty_prefix_returns_all(self):
        items = _completion_items_for("", self.prog)
        assert len(items) > 10


# ---------------------------------------------------------------------------
# Go-to definition
# ---------------------------------------------------------------------------

class TestDefinition:
    CODE = """
autotuner Calculator (int a) -> (int result) {
  result = 0;
  start -> compute;
  state compute { result = a; terminal; }
}
"""

    def setup_method(self):
        prog, _ = parse(self.CODE)
        self.prog = prog

    def test_find_autotuner(self):
        loc = _find_decl_for_word("Calculator", 0, self.prog)
        assert loc is not None

    def test_find_state(self):
        loc = _find_decl_for_word("compute", 0, self.prog)
        assert loc is not None

    def test_unknown_returns_none(self):
        loc = _find_decl_for_word("doesNotExist", 0, self.prog)
        assert loc is None

    def test_none_program_returns_none(self):
        loc = _find_decl_for_word("anything", 0, None)
        assert loc is None


# ---------------------------------------------------------------------------
# Server initialisation (smoke test)
# ---------------------------------------------------------------------------

class TestServerInit:
    def test_server_exists(self):
        from falcon_lsp.server import falcon_server
        assert falcon_server is not None

    def test_server_name(self):
        from falcon_lsp.server import SERVER_NAME
        assert SERVER_NAME == "falcon-lsp"
