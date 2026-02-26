"""Type-checker for the Falcon autotuner DSL.

Walks a parsed :class:`~falcon_lsp.ast_nodes.Program` and produces
:class:`Diagnostic` objects for semantic errors.
"""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional

from .ast_nodes import (
    AssignStmt,
    AutotunerDecl,
    BinaryExpr,
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
from .builtins import DEFAULT_REGISTRY, BuiltinRegistry


@dataclass
class Diagnostic:
    """A semantic diagnostic (error or warning)."""

    line: int
    col: int
    message: str
    severity: str = "error"  # "error" | "warning" | "hint"

    def __str__(self) -> str:
        return f"{self.line}:{self.col} [{self.severity}] {self.message}"


# A scope maps variable names to TypeDescriptors
Scope = dict[str, TypeDescriptor]


class TypeChecker:
    """Walk the AST and collect type-checking diagnostics."""

    def __init__(self, registry: Optional[BuiltinRegistry] = None) -> None:
        self._reg = registry or DEFAULT_REGISTRY
        self._diagnostics: list[Diagnostic] = []
        # Names of known autotuners and routines (for call resolution)
        self._known_autotuners: dict[str, AutotunerDecl] = {}
        self._known_routines: dict[str, RoutineDecl] = {}

    # ------------------------------------------------------------------
    # Public API
    # ------------------------------------------------------------------

    def check(self, program: Program) -> list[Diagnostic]:
        self._diagnostics = []
        # Pre-register autotuner and routine names
        for at in program.autotuners:
            self._known_autotuners[at.name] = at
        for rt in program.routines:
            self._known_routines[rt.name] = rt

        for at in program.autotuners:
            self._check_autotuner(at)
        return self._diagnostics

    # ------------------------------------------------------------------
    # Autotuner
    # ------------------------------------------------------------------

    def _check_autotuner(self, at: AutotunerDecl) -> None:
        # Build the autotuner-level scope
        input_scope: Scope = {}
        output_scope: Scope = {}
        at_scope: Scope = {}  # var_decls

        for p in at.input_params:
            input_scope[p.name] = p.type

        for p in at.output_params:
            output_scope[p.name] = p.type

        # readonly names = input params
        readonly: set[str] = set(input_scope.keys())

        # Check top-level var_decls and assigns; pass at_scope as local_scope so
        # that declarations accumulate and are visible to subsequent statements.
        for stmt in at.var_decls:
            self._check_stmt(stmt, input_scope, output_scope, at_scope, at_scope, readonly)

        # Check states
        for state in at.states:
            self._check_state(state, input_scope, output_scope, at_scope, readonly)

    # ------------------------------------------------------------------
    # State
    # ------------------------------------------------------------------

    def _check_state(
        self,
        state: StateDecl,
        input_scope: Scope,
        output_scope: Scope,
        at_scope: Scope,
        readonly: set[str],
    ) -> None:
        state_input_scope: Scope = {}
        for p in state.input_params:
            state_input_scope[p.name] = p.type

        state_local_scope: Scope = {}
        state_readonly = readonly | set(state_input_scope.keys())

        for stmt in state.body:
            self._check_stmt(
                stmt,
                input_scope,
                output_scope,
                at_scope,
                state_local_scope,
                state_readonly,
                state_input_scope=state_input_scope,
            )

    # ------------------------------------------------------------------
    # Statements
    # ------------------------------------------------------------------

    def _check_stmt(
        self,
        stmt: Stmt,
        input_scope: Scope,
        output_scope: Scope,
        at_scope: Scope,
        local_scope: Scope,
        readonly: set[str],
        state_input_scope: Optional[Scope] = None,
    ) -> None:
        si = state_input_scope or {}

        def lookup(name: str) -> Optional[TypeDescriptor]:
            return (
                local_scope.get(name)
                or si.get(name)
                or at_scope.get(name)
                or output_scope.get(name)
                or input_scope.get(name)
            )

        if isinstance(stmt, VarDeclStmt):
            # Check redeclaration
            combined = {**input_scope, **output_scope, **at_scope, **si}
            if stmt.name in combined or stmt.name in local_scope:
                self._warn(stmt.line, stmt.column,
                           f"redeclaration of variable '{stmt.name}'")
            # Check initializer type
            if stmt.initializer:
                init_type = self._infer_expr(
                    stmt.initializer, input_scope, output_scope,
                    at_scope, local_scope, si,
                )
                if init_type and not init_type.is_compatible_with(stmt.type):
                    self._error(
                        stmt.line, stmt.column,
                        f"type mismatch: cannot assign {init_type} to {stmt.type}",
                    )
            local_scope[stmt.name] = stmt.type

        elif isinstance(stmt, AssignStmt):
            val_type = self._infer_expr(
                stmt.value, input_scope, output_scope, at_scope, local_scope, si
            ) if stmt.value else None

            # Tuple-unpacking: check result count
            if val_type and val_type.base_type == "tuple":
                if len(val_type.tuple_elements) != len(stmt.targets):
                    self._error(
                        stmt.line, stmt.column,
                        f"tuple unpacking: expected {len(val_type.tuple_elements)} "
                        f"targets, got {len(stmt.targets)}",
                    )

            for i, target in enumerate(stmt.targets):
                t = lookup(target)
                if t is None:
                    self._error(stmt.line, stmt.column,
                                f"undeclared variable '{target}'")
                    continue
                if target in readonly:
                    self._error(stmt.line, stmt.column,
                                f"cannot assign to read-only parameter '{target}'")
                    continue
                if val_type:
                    assign_type = (
                        val_type.tuple_elements[i]
                        if val_type.base_type == "tuple"
                        and i < len(val_type.tuple_elements)
                        else val_type
                    )
                    if not assign_type.is_compatible_with(t):
                        self._error(
                            stmt.line, stmt.column,
                            f"type mismatch: cannot assign {assign_type} to {t} "
                            f"(target '{target}')",
                        )

        elif isinstance(stmt, ExprStmt):
            if stmt.expr:
                self._infer_expr(
                    stmt.expr, input_scope, output_scope, at_scope, local_scope, si
                )

        elif isinstance(stmt, IfStmt):
            if stmt.condition:
                cond_type = self._infer_expr(
                    stmt.condition, input_scope, output_scope,
                    at_scope, local_scope, si,
                )
            for s in stmt.then_body:
                self._check_stmt(s, input_scope, output_scope,
                                  at_scope, local_scope, readonly, si)
            for s in stmt.else_body:
                self._check_stmt(s, input_scope, output_scope,
                                  at_scope, local_scope, readonly, si)

        elif isinstance(stmt, (TransitionStmt, TerminalStmt)):
            pass  # no type-level checking needed here

    # ------------------------------------------------------------------
    # Expressions
    # ------------------------------------------------------------------

    def _infer_expr(
        self,
        expr: Expr,
        input_scope: Scope,
        output_scope: Scope,
        at_scope: Scope,
        local_scope: Scope,
        state_input_scope: Scope,
    ) -> Optional[TypeDescriptor]:
        """Infer and return the type of *expr*, attaching it as ``inferred_type``."""

        def lookup(name: str) -> Optional[TypeDescriptor]:
            return (
                local_scope.get(name)
                or state_input_scope.get(name)
                or at_scope.get(name)
                or output_scope.get(name)
                or input_scope.get(name)
            )

        t: Optional[TypeDescriptor] = None

        if isinstance(expr, LiteralExpr):
            if isinstance(expr.value, bool):
                t = TypeDescriptor(base_type="bool")
            elif isinstance(expr.value, int):
                t = TypeDescriptor(base_type="int")
            elif isinstance(expr.value, float):
                t = TypeDescriptor(base_type="float")
            elif isinstance(expr.value, str):
                t = TypeDescriptor(base_type="string")

        elif isinstance(expr, NilLiteralExpr):
            t = TypeDescriptor(base_type="nil")

        elif isinstance(expr, VarExpr):
            found = lookup(expr.name)
            if found is None:
                # Could be a known autotuner/routine being called elsewhere
                if expr.name not in self._known_autotuners and \
                   expr.name not in self._known_routines:
                    self._error(expr.line, expr.column,
                                f"undeclared variable '{expr.name}'")
            t = found

        elif isinstance(expr, BinaryExpr):
            lt = self._infer_expr(expr.left, input_scope, output_scope,
                                   at_scope, local_scope, state_input_scope) if expr.left else None
            rt = self._infer_expr(expr.right, input_scope, output_scope,
                                   at_scope, local_scope, state_input_scope) if expr.right else None
            if expr.op in ("==", "!=", "<", ">", "<=", ">=", "&&", "||"):
                t = TypeDescriptor(base_type="bool")
            else:
                # Arithmetic: use left operand type (simple heuristic)
                t = lt

        elif isinstance(expr, UnaryExpr):
            ot = self._infer_expr(expr.operand, input_scope, output_scope,
                                   at_scope, local_scope, state_input_scope) if expr.operand else None
            if expr.op == "!":
                t = TypeDescriptor(base_type="bool")
            else:
                t = ot

        elif isinstance(expr, MemberExpr):
            obj_type = self._infer_expr(expr.obj, input_scope, output_scope,
                                         at_scope, local_scope, state_input_scope) if expr.obj else None
            if obj_type:
                spec = self._reg.get_member(obj_type.base_type, expr.member)
                if spec:
                    t = spec.type
                else:
                    self._warn(expr.line, expr.column,
                               f"unknown member '{expr.member}' on type '{obj_type.base_type}'")

        elif isinstance(expr, MethodCallExpr):
            obj_type = self._infer_expr(expr.obj, input_scope, output_scope,
                                         at_scope, local_scope, state_input_scope) if expr.obj else None
            if obj_type:
                spec = self._reg.get_member(obj_type.base_type, expr.method)
                if spec and spec.is_method:
                    t = spec.type
                else:
                    self._warn(expr.line, expr.column,
                               f"unknown method '{expr.method}' on type '{obj_type.base_type}'")

        elif isinstance(expr, IndexExpr):
            obj_type = self._infer_expr(expr.obj, input_scope, output_scope,
                                         at_scope, local_scope, state_input_scope) if expr.obj else None
            if expr.index:
                self._infer_expr(expr.index, input_scope, output_scope,
                                  at_scope, local_scope, state_input_scope)
            if obj_type and obj_type.base_type == "Connections":
                t = TypeDescriptor(base_type="Connection")
            elif obj_type:
                t = obj_type

        elif isinstance(expr, CallExpr):
            t = self._infer_call(expr, input_scope, output_scope,
                                  at_scope, local_scope, state_input_scope)

        expr.inferred_type = t
        return t

    def _infer_call(
        self,
        expr: CallExpr,
        input_scope: Scope,
        output_scope: Scope,
        at_scope: Scope,
        local_scope: Scope,
        state_input_scope: Scope,
    ) -> Optional[TypeDescriptor]:
        """Infer the return type of a function call expression."""
        # Infer arg types (for side-effects / nested diagnostics)
        for arg in expr.args:
            if arg.value:
                self._infer_expr(arg.value, input_scope, output_scope,
                                  at_scope, local_scope, state_input_scope)

        # Built-in function
        sig = self._reg.get_function(expr.name)
        if sig:
            return sig.return_type()

        # Known autotuner call
        at = self._known_autotuners.get(expr.name)
        if at:
            out_types = [p.type for p in at.output_params]
            if len(out_types) == 1:
                return out_types[0]
            if out_types:
                return TypeDescriptor.make_tuple(out_types)
            return TypeDescriptor(base_type="void")

        # Known routine call
        rt = self._known_routines.get(expr.name)
        if rt:
            out_types = [p.type for p in rt.output_params]
            if len(out_types) == 1:
                return out_types[0]
            if out_types:
                return TypeDescriptor.make_tuple(out_types)
            return TypeDescriptor(base_type="void")

        # Unknown function – warn but don't error (could be user-defined)
        self._warn(expr.line, expr.column,
                   f"unknown function '{expr.name}'")
        return None

    # ------------------------------------------------------------------
    # Helpers
    # ------------------------------------------------------------------

    def _error(self, line: int, col: int, msg: str) -> None:
        self._diagnostics.append(Diagnostic(line, col, msg, severity="error"))

    def _warn(self, line: int, col: int, msg: str) -> None:
        self._diagnostics.append(Diagnostic(line, col, msg, severity="warning"))
