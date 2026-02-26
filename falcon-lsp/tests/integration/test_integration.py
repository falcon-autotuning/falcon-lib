"""Integration tests: parse all real .fal files from the test suite."""

from __future__ import annotations

import os
from pathlib import Path

import pytest

from falcon_lsp.parser import parse, ParseError
from falcon_lsp.type_checker import TypeChecker

# Path to the actual .fal test fixtures
_FAL_DIR = Path(__file__).parents[3] / "autotuner" / "tests" / "test-autotuners"

# Files that are intentionally broken (contain syntax we don't support or
# have deliberate errors) – we still parse them but don't require zero errors.
_KNOWN_BROKEN = {
    "simple-error.fal",       # uses Error::msg(...) syntax not in grammar
    "fatal-simple-error.fal", # uses terminal FatalError::msg(...) syntax
    "routine-nest.fal",       # uses 'requires :' instead of 'uses'
    "generic-iteration-test.fal",  # broken if/else syntax in original
    "conditional-nest.fal",   # uses colon-syntax (name:value) for transition params
}

# Collect all .fal files
def _all_fal_files() -> list[Path]:
    if not _FAL_DIR.exists():
        return []
    return sorted(_FAL_DIR.rglob("*.fal"))


@pytest.mark.parametrize("fal_file", _all_fal_files(), ids=lambda p: p.name)
def test_parse_does_not_crash(fal_file: Path) -> None:
    """Every .fal file should parse without throwing an exception."""
    text = fal_file.read_text()
    prog, errors = parse(text)
    # parse() should always return a Program (may be empty on heavy errors)
    assert prog is not None


@pytest.mark.parametrize(
    "fal_file",
    [p for p in _all_fal_files() if p.name not in _KNOWN_BROKEN],
    ids=lambda p: p.name,
)
def test_valid_files_parse_without_errors(fal_file: Path) -> None:
    """All non-broken .fal files should parse without parse errors."""
    text = fal_file.read_text()
    prog, errors = parse(text)
    parse_errors_str = "\n".join(str(e) for e in errors)
    assert errors == [], (
        f"{fal_file.name} produced parse errors:\n{parse_errors_str}"
    )


@pytest.mark.parametrize(
    "fal_file",
    [p for p in _all_fal_files() if p.name not in _KNOWN_BROKEN],
    ids=lambda p: p.name,
)
def test_valid_files_type_check_no_crash(fal_file: Path) -> None:
    """Type-checker should not crash on any valid .fal file."""
    text = fal_file.read_text()
    prog, _ = parse(text)
    if prog is None:
        pytest.skip("Parse returned None")
    checker = TypeChecker()
    diags = checker.check(prog)
    # Just ensure no exception is raised; some warnings are ok


# ---------------------------------------------------------------------------
# Basic features: specific symbol checks
# ---------------------------------------------------------------------------

class TestCalculatorSymbols:
    """Verify key symbols for the calculator.fal fixture."""

    @pytest.fixture(autouse=True)
    def load(self):
        path = _FAL_DIR / "basic_features" / "calculator.fal"
        if not path.exists():
            pytest.skip("calculator.fal not found")
        text = path.read_text()
        self.prog, self.errors = parse(text)

    def test_no_parse_errors(self):
        assert self.errors == []

    def test_autotuner_name(self):
        assert self.prog.autotuners[0].name == "Calculator"

    def test_input_params(self):
        at = self.prog.autotuners[0]
        names = [p.name for p in at.input_params]
        assert "a" in names
        assert "b" in names

    def test_output_params(self):
        at = self.prog.autotuners[0]
        names = [p.name for p in at.output_params]
        assert "sum" in names
        assert "product" in names

    def test_states(self):
        at = self.prog.autotuners[0]
        state_names = [s.name for s in at.states]
        assert "calculate" in state_names
        assert "done" in state_names

    def test_type_check_no_errors(self):
        checker = TypeChecker()
        diags = [d for d in checker.check(self.prog) if d.severity == "error"]
        assert diags == [], "\n".join(str(d) for d in diags)


class TestConditionalBranchSymbols:
    """Verify key symbols for conditional-branch.fal."""

    @pytest.fixture(autouse=True)
    def load(self):
        path = _FAL_DIR / "basic_features" / "conditional-branch.fal"
        if not path.exists():
            pytest.skip("conditional-branch.fal not found")
        text = path.read_text()
        self.prog, self.errors = parse(text)

    def test_no_parse_errors(self):
        assert self.errors == []

    def test_entry_state(self):
        assert self.prog.autotuners[0].entry_state == "check"

    def test_all_states_present(self):
        state_names = {s.name for s in self.prog.autotuners[0].states}
        assert state_names >= {"check", "high", "low", "done"}


class TestStateReadSymbols:
    """Verify state-read.fal from database_access fixtures."""

    @pytest.fixture(autouse=True)
    def load(self):
        path = _FAL_DIR / "database_access" / "state-read.fal"
        if not path.exists():
            pytest.skip("state-read.fal not found")
        text = path.read_text()
        self.prog, self.errors = parse(text)

    def test_no_parse_errors(self):
        assert self.errors == [], "\n".join(str(e) for e in self.errors)

    def test_readLatest_present(self):
        """init state should contain a readLatest call."""
        at = self.prog.autotuners[0]
        init_state = next((s for s in at.states if s.name == "init"), None)
        assert init_state is not None
        from falcon_lsp.ast_nodes import AssignStmt, CallExpr
        calls = [
            s.value for s in init_state.body
            if isinstance(s, AssignStmt) and isinstance(s.value, CallExpr)
        ]
        assert any(c.name == "readLatest" for c in calls)
