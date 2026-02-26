"""AST node dataclasses for the Falcon autotuner DSL."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Optional


# ---------------------------------------------------------------------------
# Types
# ---------------------------------------------------------------------------

@dataclass
class TypeDescriptor:
    """Describes a Falcon type.

    *base_type* is one of: int, float, bool, string, Quantity, Config,
    Connection, Connections, Gname, Error, nil, void, tuple, union.
    """

    base_type: str = "void"
    tuple_elements: list["TypeDescriptor"] = field(default_factory=list)
    union_types: list["TypeDescriptor"] = field(default_factory=list)
    # For Error variants
    error_variant: str = ""

    def __repr__(self) -> str:  # pragma: no cover
        if self.base_type == "tuple" and self.tuple_elements:
            inner = ", ".join(repr(t) for t in self.tuple_elements)
            return f"({inner})"
        if self.base_type == "union" and self.union_types:
            inner = "|".join(repr(t) for t in self.union_types)
            return f"union<{inner}>"
        return self.base_type

    def __str__(self) -> str:
        return repr(self)

    @staticmethod
    def make_union(types: list["TypeDescriptor"]) -> "TypeDescriptor":
        return TypeDescriptor(base_type="union", union_types=types)

    @staticmethod
    def make_tuple(types: list["TypeDescriptor"]) -> "TypeDescriptor":
        return TypeDescriptor(base_type="tuple", tuple_elements=types)

    def is_compatible_with(self, other: "TypeDescriptor") -> bool:
        """Return True if *self* can be assigned to *other*."""
        if self.base_type == "nil":
            # nil is compatible with Error and union types
            return other.base_type in ("Error", "nil") or other.base_type == "union"
        if self.base_type == other.base_type:
            return True
        if other.base_type == "union":
            return any(self.is_compatible_with(t) for t in other.union_types)
        # A union type is compatible with any of its member types (assignment from union)
        if self.base_type == "union":
            return any(t.is_compatible_with(other) for t in self.union_types)
        return False


# ---------------------------------------------------------------------------
# Parameter declarations
# ---------------------------------------------------------------------------

@dataclass
class ParamDecl:
    """A typed parameter declaration (e.g. ``int a`` or ``int a = 0``)."""

    type: TypeDescriptor
    name: str
    default_value: Optional["Expr"] = None
    line: int = 0
    column: int = 0


# ---------------------------------------------------------------------------
# Expressions  (base + concrete)
# ---------------------------------------------------------------------------

@dataclass
class Expr:
    """Abstract base for all expression nodes."""

    line: int = 0
    column: int = 0
    # Inferred type, filled by the type-checker
    inferred_type: Optional[TypeDescriptor] = field(default=None, compare=False)


@dataclass
class LiteralExpr(Expr):
    """An integer, float, bool, or string literal."""

    value: int | float | bool | str = 0


@dataclass
class NilLiteralExpr(Expr):
    """The ``nil`` literal."""


@dataclass
class VarExpr(Expr):
    """A variable reference."""

    name: str = ""


@dataclass
class BinaryExpr(Expr):
    """A binary operation."""

    op: str = ""
    left: Optional[Expr] = None
    right: Optional[Expr] = None


@dataclass
class UnaryExpr(Expr):
    """A unary operation (``!`` or unary ``-``)."""

    op: str = ""
    operand: Optional[Expr] = None


@dataclass
class MemberExpr(Expr):
    """A member access (``obj.field``)."""

    obj: Optional[Expr] = None
    member: str = ""


@dataclass
class MethodCallExpr(Expr):
    """A method call (``obj.method(args)``)."""

    obj: Optional[Expr] = None
    method: str = ""
    args: list[Expr] = field(default_factory=list)


@dataclass
class IndexExpr(Expr):
    """Array indexing (``arr[idx]``)."""

    obj: Optional[Expr] = None
    index: Optional[Expr] = None


@dataclass
class CallArg:
    """A single argument in a function call (positional or named)."""

    value: Optional[Expr] = None
    keyword: Optional[str] = None  # None means positional


@dataclass
class CallExpr(Expr):
    """A free-function call (``f(args)``)."""

    name: str = ""
    args: list[CallArg] = field(default_factory=list)


# ---------------------------------------------------------------------------
# Statements  (base + concrete)
# ---------------------------------------------------------------------------

@dataclass
class Stmt:
    """Abstract base for all statement nodes."""

    line: int = 0
    column: int = 0


@dataclass
class VarDeclStmt(Stmt):
    """A local variable declaration."""

    type: TypeDescriptor = field(default_factory=TypeDescriptor)
    name: str = ""
    initializer: Optional[Expr] = None


@dataclass
class AssignStmt(Stmt):
    """An assignment (possibly multi-target for tuple unpacking)."""

    targets: list[str] = field(default_factory=list)
    value: Optional[Expr] = None


@dataclass
class ExprStmt(Stmt):
    """An expression used as a statement (e.g. a function call)."""

    expr: Optional[Expr] = None


@dataclass
class IfStmt(Stmt):
    """An if / elif / else construct."""

    condition: Optional[Expr] = None
    then_body: list[Stmt] = field(default_factory=list)
    else_body: list[Stmt] = field(default_factory=list)  # may contain nested IfStmt


@dataclass
class TransitionStmt(Stmt):
    """A state transition (``-> state_name``)."""

    target: str = ""
    params: list[Expr] = field(default_factory=list)


@dataclass
class TerminalStmt(Stmt):
    """The ``terminal`` statement."""


# ---------------------------------------------------------------------------
# Top-level declarations
# ---------------------------------------------------------------------------

@dataclass
class StateDecl:
    """A ``state`` block inside an autotuner."""

    name: str = ""
    input_params: list[ParamDecl] = field(default_factory=list)
    body: list[Stmt] = field(default_factory=list)
    line: int = 0
    column: int = 0


@dataclass
class AutotunerDecl:
    """An ``autotuner`` declaration."""

    name: str = ""
    input_params: list[ParamDecl] = field(default_factory=list)
    output_params: list[ParamDecl] = field(default_factory=list)
    uses: list[str] = field(default_factory=list)
    var_decls: list[Stmt] = field(default_factory=list)
    entry_state: str = ""
    entry_params: list[Expr] = field(default_factory=list)
    states: list[StateDecl] = field(default_factory=list)
    line: int = 0
    column: int = 0


@dataclass
class RoutineDecl:
    """A ``routine`` declaration (external function stub)."""

    name: str = ""
    input_params: list[ParamDecl] = field(default_factory=list)
    output_params: list[ParamDecl] = field(default_factory=list)
    line: int = 0
    column: int = 0


@dataclass
class Program:
    """The root of an AST."""

    autotuners: list[AutotunerDecl] = field(default_factory=list)
    routines: list[RoutineDecl] = field(default_factory=list)
