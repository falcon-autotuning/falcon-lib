"""Tests for the Falcon DSL type-checker."""

import pytest
from falcon_lsp.parser import parse
from falcon_lsp.type_checker import TypeChecker, Diagnostic


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def check(code: str) -> list[Diagnostic]:
    prog, _ = parse(code)
    if prog is None:
        return []
    return TypeChecker().check(prog)


def errors_only(diags: list[Diagnostic]) -> list[Diagnostic]:
    return [d for d in diags if d.severity == "error"]


# ---------------------------------------------------------------------------
# Valid programs
# ---------------------------------------------------------------------------

class TestValidPrograms:
    def test_calculator_no_errors(self):
        code = """
autotuner Calculator (int a, int b) -> (int sum, int product) {
  sum = 0;
  product = 0;
  start -> calculate;
  state calculate {
    sum = a + b;
    product = a * b;
    -> done;
  }
  state done { terminal; }
}
"""
        diags = errors_only(check(code))
        assert diags == [], "\n".join(str(d) for d in diags)

    def test_bool_assign(self):
        code = """
autotuner X -> (bool r) {
  r = false;
  start -> s;
  state s { r = true; terminal; }
}
"""
        diags = errors_only(check(code))
        assert diags == []

    def test_nil_to_error(self):
        code = """
autotuner X -> (bool r) {
  r = false;
  Error e = nil;
  start -> s;
  state s { terminal; }
}
"""
        diags = errors_only(check(code))
        assert diags == []

    def test_string_assign(self):
        code = """
autotuner X -> (string r) {
  r = "";
  start -> s;
  state s { r = "hello"; terminal; }
}
"""
        diags = errors_only(check(code))
        assert diags == []


# ---------------------------------------------------------------------------
# Undeclared variables
# ---------------------------------------------------------------------------

class TestUndeclaredVariables:
    def test_undeclared_in_state(self):
        code = """
autotuner X -> (int r) {
  r = 0;
  start -> s;
  state s {
    r = undeclaredVar;
    terminal;
  }
}
"""
        diags = errors_only(check(code))
        assert any("undeclaredVar" in d.message for d in diags)

    def test_declared_at_autotuner_level(self):
        code = """
autotuner X -> (int r) {
  int tmp = 0;
  r = tmp;
  start -> s;
  state s { terminal; }
}
"""
        diags = errors_only(check(code))
        assert diags == []

    def test_declared_in_state_local(self):
        code = """
autotuner X -> (int r) {
  r = 0;
  start -> s;
  state s {
    int local = 5;
    r = local;
    terminal;
  }
}
"""
        diags = errors_only(check(code))
        assert diags == []


# ---------------------------------------------------------------------------
# Read-only input parameters
# ---------------------------------------------------------------------------

class TestReadOnly:
    def test_cannot_assign_to_input_param(self):
        code = """
autotuner X (int a) -> (int r) {
  r = 0;
  start -> s;
  state s {
    a = 5;
    terminal;
  }
}
"""
        diags = errors_only(check(code))
        assert any("read-only" in d.message.lower() or "input" in d.message.lower()
                   for d in diags)

    def test_can_read_input_param(self):
        code = """
autotuner X (int a) -> (int r) {
  r = 0;
  start -> s;
  state s {
    r = a;
    terminal;
  }
}
"""
        diags = errors_only(check(code))
        assert diags == []


# ---------------------------------------------------------------------------
# Type inference for expressions
# ---------------------------------------------------------------------------

class TestTypeInference:
    def test_int_literal_type(self):
        from falcon_lsp.ast_nodes import LiteralExpr
        from falcon_lsp.lexer import tokenize
        from falcon_lsp.parser import Parser
        tokens = tokenize("autotuner X -> (int r) { r = 42; start -> s; state s { terminal; } }")
        prog, _ = Parser(tokens).parse()
        tc = TypeChecker()
        tc.check(prog)
        # Find the assignment 'r = 42' and check the literal type
        stmt = prog.autotuners[0].var_decls[0]
        from falcon_lsp.ast_nodes import AssignStmt
        assert isinstance(stmt, AssignStmt)
        assert stmt.value.inferred_type is not None
        assert stmt.value.inferred_type.base_type == "int"

    def test_bool_op_returns_bool(self):
        code = """
autotuner X (int a) -> (bool r) {
  r = false;
  start -> s;
  state s { r = a > 0; terminal; }
}
"""
        prog, _ = parse(code)
        tc = TypeChecker()
        tc.check(prog)
        state = prog.autotuners[0].states[0]
        from falcon_lsp.ast_nodes import AssignStmt
        assigns = [s for s in state.body if isinstance(s, AssignStmt)]
        assert assigns[0].value.inferred_type.base_type == "bool"


# ---------------------------------------------------------------------------
# Built-in function validation
# ---------------------------------------------------------------------------

class TestBuiltins:
    def test_logInfo_call_no_error(self):
        code = """
autotuner X -> (bool r) {
  r = false;
  start -> s;
  state s { logInfo("hello"); terminal; }
}
"""
        diags = errors_only(check(code))
        assert diags == []

    def test_readLatest_return_type_inferred(self):
        code = """
autotuner X -> (bool c) {
  c = false;
  Error e = nil;
  start -> s;
  state s {
    c, e = readLatest(scope="g", name="x", extra="default");
    terminal;
  }
}
"""
        diags = errors_only(check(code))
        # Type-checker should not error on a valid readLatest usage
        # (c can accept the union return, e gets Error)
        assert not any("undeclared" in d.message for d in diags)

    def test_errorMsg_returns_error_type(self):
        code = """
autotuner X -> (Error err) {
  err = nil;
  start -> s;
  state s {
    err = errorMsg("oops");
    terminal;
  }
}
"""
        diags = errors_only(check(code))
        assert diags == []
