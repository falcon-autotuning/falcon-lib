"""Tests for the Falcon DSL parser."""

import pytest
from falcon_lsp.parser import parse, ParseError
from falcon_lsp.ast_nodes import (
    AssignStmt,
    AutotunerDecl,
    BinaryExpr,
    CallExpr,
    ExprStmt,
    IfStmt,
    LiteralExpr,
    Program,
    RoutineDecl,
    StateDecl,
    TerminalStmt,
    TransitionStmt,
    VarDeclStmt,
    VarExpr,
)


# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def assert_no_fatal(errors: list[ParseError], text: str = "") -> None:
    """Fail if there are any parse errors (for valid programs)."""
    if errors:
        msgs = "\n".join(str(e) for e in errors)
        pytest.fail(f"Unexpected parse errors:\n{msgs}\nSource:\n{text}")


# ---------------------------------------------------------------------------
# Basic autotuner
# ---------------------------------------------------------------------------

class TestSimpleAutotuner:
    CALC = """
autotuner Calculator (int a, int b) -> (int sum, int product) {
  sum = 0;
  product = 0;
  start -> calculate;
  state calculate {
    sum = a + b;
    product = a * b;
    -> done;
  }
  state done {
    terminal;
  }
}
"""

    def test_parse_succeeds(self):
        prog, errors = parse(self.CALC)
        assert prog is not None
        assert errors == []

    def test_autotuner_name(self):
        prog, _ = parse(self.CALC)
        assert prog.autotuners[0].name == "Calculator"

    def test_input_params(self):
        prog, _ = parse(self.CALC)
        at = prog.autotuners[0]
        assert len(at.input_params) == 2
        assert at.input_params[0].name == "a"
        assert at.input_params[0].type.base_type == "int"

    def test_output_params(self):
        prog, _ = parse(self.CALC)
        at = prog.autotuners[0]
        assert len(at.output_params) == 2
        assert at.output_params[0].name == "sum"

    def test_entry_state(self):
        prog, _ = parse(self.CALC)
        assert prog.autotuners[0].entry_state == "calculate"

    def test_states(self):
        prog, _ = parse(self.CALC)
        at = prog.autotuners[0]
        assert len(at.states) == 2
        state_names = [s.name for s in at.states]
        assert "calculate" in state_names
        assert "done" in state_names


# ---------------------------------------------------------------------------
# State body statements
# ---------------------------------------------------------------------------

class TestStatements:
    def test_var_decl(self):
        code = """
autotuner X -> (int out) {
  out = 0;
  start -> s;
  state s {
    int tmp = 5;
    terminal;
  }
}
"""
        prog, errors = parse(code)
        assert prog is not None
        state = prog.autotuners[0].states[0]
        assert any(isinstance(s, VarDeclStmt) and s.name == "tmp" for s in state.body)

    def test_assignment(self):
        code = """
autotuner X -> (int out) {
  out = 0;
  start -> s;
  state s {
    out = 42;
    terminal;
  }
}
"""
        prog, _ = parse(code)
        state = prog.autotuners[0].states[0]
        assigns = [s for s in state.body if isinstance(s, AssignStmt)]
        assert len(assigns) == 1
        assert assigns[0].targets == ["out"]

    def test_transition(self):
        code = """
autotuner X -> (bool b) {
  b = false;
  start -> init;
  state init {
    -> done;
  }
  state done { terminal; }
}
"""
        prog, _ = parse(code)
        state = prog.autotuners[0].states[0]
        trans = [s for s in state.body if isinstance(s, TransitionStmt)]
        assert len(trans) == 1
        assert trans[0].target == "done"

    def test_terminal(self):
        code = """
autotuner X -> (bool b) {
  b = false;
  start -> s;
  state s { terminal; }
}
"""
        prog, _ = parse(code)
        state = prog.autotuners[0].states[0]
        assert any(isinstance(s, TerminalStmt) for s in state.body)


# ---------------------------------------------------------------------------
# If / elif / else
# ---------------------------------------------------------------------------

class TestIfElif:
    CODE = """
autotuner Branch (int v) -> (string r) {
  r = "";
  start -> check;
  state check {
    if (v < 0) {
      -> neg;
    }
    elif (v == 0) {
      -> zero;
    }
    else {
      -> pos;
    }
  }
  state neg { r = "neg"; terminal; }
  state zero { r = "zero"; terminal; }
  state pos { r = "pos"; terminal; }
}
"""

    def test_parse_succeeds(self):
        prog, errors = parse(self.CODE)
        assert prog is not None, f"Errors: {errors}"

    def test_if_structure(self):
        prog, _ = parse(self.CODE)
        state = prog.autotuners[0].states[0]
        ifs = [s for s in state.body if isinstance(s, IfStmt)]
        assert len(ifs) == 1
        assert ifs[0].condition is not None

    def test_elif_in_else_body(self):
        prog, _ = parse(self.CODE)
        state = prog.autotuners[0].states[0]
        ifs = [s for s in state.body if isinstance(s, IfStmt)]
        # else_body contains the elif-converted IfStmt
        assert len(ifs[0].else_body) == 1
        assert isinstance(ifs[0].else_body[0], IfStmt)


# ---------------------------------------------------------------------------
# Expressions
# ---------------------------------------------------------------------------

class TestExpressions:
    def test_binary_arithmetic(self):
        code = """
autotuner X -> (int r) {
  r = 0;
  start -> s;
  state s { r = 1 + 2; terminal; }
}
"""
        prog, _ = parse(code)
        state = prog.autotuners[0].states[0]
        assigns = [s for s in state.body if isinstance(s, AssignStmt)]
        assert isinstance(assigns[0].value, BinaryExpr)
        assert assigns[0].value.op == "+"

    def test_function_call(self):
        code = """
autotuner X -> (int r) {
  r = 0;
  start -> s;
  state s {
    logInfo("hello");
    terminal;
  }
}
"""
        prog, _ = parse(code)
        state = prog.autotuners[0].states[0]
        exprstmts = [s for s in state.body if isinstance(s, ExprStmt)]
        assert len(exprstmts) >= 1
        assert isinstance(exprstmts[0].expr, CallExpr)
        assert exprstmts[0].expr.name == "logInfo"

    def test_named_arg_call(self):
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
        prog, _ = parse(code)
        state = prog.autotuners[0].states[0]
        assigns = [s for s in state.body if isinstance(s, AssignStmt)]
        assert len(assigns) == 1
        call = assigns[0].value
        assert isinstance(call, CallExpr)
        assert call.name == "readLatest"
        keywords = [a.keyword for a in call.args if a.keyword]
        assert "scope" in keywords


# ---------------------------------------------------------------------------
# Routines
# ---------------------------------------------------------------------------

class TestRoutines:
    def test_parse_routine(self):
        code = """
autotuner X -> (bool b) {
  b = false;
  start -> s;
  state s { terminal; }
}
routine Adder (int a, int b) -> (int out, Error err)
"""
        prog, errors = parse(code)
        assert prog is not None
        assert len(prog.routines) == 1
        assert prog.routines[0].name == "Adder"

    def test_routine_params(self):
        code = """
autotuner X -> (bool b) {
  b = false;
  start -> s;
  state s { terminal; }
}
routine Adder (int a, int b) -> (int out, Error err)
"""
        prog, _ = parse(code)
        rt = prog.routines[0]
        assert len(rt.input_params) == 2
        assert rt.input_params[0].name == "a"
        assert len(rt.output_params) == 2


# ---------------------------------------------------------------------------
# Error recovery
# ---------------------------------------------------------------------------

class TestErrorRecovery:
    def test_missing_semicolon_continues(self):
        # Missing semicolon – parser should still produce a partial AST
        code = """
autotuner X -> (bool b) {
  b = false
  start -> s;
  state s { terminal; }
}
"""
        prog, errors = parse(code)
        # Errors should be reported but program should not be None
        assert prog is not None

    def test_unknown_token(self):
        code = """
autotuner X -> (bool b) {
  b = false;
  start -> s;
  state s { @invalid; terminal; }
}
"""
        prog, errors = parse(code)
        assert prog is not None  # should not crash

    def test_no_programs_returns_empty_program(self):
        prog, errors = parse("")
        assert prog is not None
        assert prog.autotuners == []
        assert prog.routines == []


# ---------------------------------------------------------------------------
# Uses clause
# ---------------------------------------------------------------------------

class TestUsesClause:
    def test_uses_parsed(self):
        code = """
autotuner X -> (bool b) {
  uses Foo, Bar;
  b = false;
  start -> s;
  state s { terminal; }
}
"""
        prog, errors = parse(code)
        assert prog is not None
        at = prog.autotuners[0]
        assert "Foo" in at.uses
        assert "Bar" in at.uses
