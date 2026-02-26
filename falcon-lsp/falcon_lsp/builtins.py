"""Built-in function and type-member registry for the Falcon DSL."""

from __future__ import annotations

from dataclasses import dataclass, field
from typing import Optional

from .ast_nodes import TypeDescriptor


@dataclass
class ParamSpec:
    """Specification for a single parameter or return value."""

    name: str
    type: TypeDescriptor
    required: bool = True  # False = optional / named


@dataclass
class BuiltinSignature:
    """Signature of a built-in (or routine) function."""

    name: str
    params: list[ParamSpec] = field(default_factory=list)
    returns: list[ParamSpec] = field(default_factory=list)
    uses_named_args: bool = False  # True when extra kwargs are allowed

    def display(self) -> str:
        """Return a human-readable signature string."""
        param_str = ", ".join(
            f"{p.name}: {p.type}" + ("" if p.required else "?")
            for p in self.params
        )
        if self.uses_named_args:
            param_str += ", ..." if param_str else "..."
        if len(self.returns) == 0:
            ret_str = "void"
        elif len(self.returns) == 1:
            ret_str = str(self.returns[0].type)
        else:
            ret_str = "(" + ", ".join(str(r.type) for r in self.returns) + ")"
        return f"{self.name}({param_str}) -> {ret_str}"

    def return_type(self) -> TypeDescriptor:
        """Return the effective return TypeDescriptor."""
        if not self.returns:
            return TypeDescriptor(base_type="void")
        if len(self.returns) == 1:
            return self.returns[0].type
        return TypeDescriptor.make_tuple([r.type for r in self.returns])


@dataclass
class MemberSpec:
    """Description of a field or method on a Falcon type."""

    name: str
    type: TypeDescriptor
    is_method: bool = False
    method_params: list[ParamSpec] = field(default_factory=list)

    def display(self) -> str:
        if self.is_method:
            params = ", ".join(f"{p.name}: {p.type}" for p in self.method_params)
            return f"{self.name}({params}) -> {self.type}"
        return f"{self.name}: {self.type}"


class BuiltinRegistry:
    """Registry of built-in functions and type members."""

    def __init__(self) -> None:
        self._functions: dict[str, BuiltinSignature] = {}
        self._members: dict[str, list[MemberSpec]] = {}

    # ------------------------------------------------------------------
    # Registration
    # ------------------------------------------------------------------

    def register(self, sig: BuiltinSignature) -> None:
        self._functions[sig.name] = sig

    def register_member(self, type_name: str, spec: MemberSpec) -> None:
        self._members.setdefault(type_name, []).append(spec)

    # ------------------------------------------------------------------
    # Lookup
    # ------------------------------------------------------------------

    def get_function(self, name: str) -> Optional[BuiltinSignature]:
        return self._functions.get(name)

    def get_members(self, type_name: str) -> list[MemberSpec]:
        return self._members.get(type_name, [])

    def get_member(self, type_name: str, member_name: str) -> Optional[MemberSpec]:
        for m in self._members.get(type_name, []):
            if m.name == member_name:
                return m
        return None

    def all_function_names(self) -> list[str]:
        return list(self._functions.keys())


def create_default_registry() -> BuiltinRegistry:
    """Build and return the default built-in registry matching BuiltinRegistry.cpp."""
    reg = BuiltinRegistry()

    # ------------------------------------------------------------------ logging
    for fn in ("logInfo", "logWarn", "logError"):
        reg.register(BuiltinSignature(
            name=fn,
            params=[ParamSpec("format", TypeDescriptor(base_type="string"), required=True)],
            returns=[ParamSpec("out", TypeDescriptor(base_type="void"))],
            uses_named_args=True,
        ))

    # -------------------------------------------------------- error construction
    for fn in ("errorMsg", "fatalErrorMsg"):
        reg.register(BuiltinSignature(
            name=fn,
            params=[ParamSpec("message", TypeDescriptor(base_type="string"), required=True)],
            returns=[ParamSpec("out", TypeDescriptor(base_type="Error"))],
        ))

    # ---------------------------------------------------------- database: readLatest
    _optional_db_params = [
        ParamSpec("barrier_gate", TypeDescriptor(base_type="string"), required=False),
        ParamSpec("plunger_gate", TypeDescriptor(base_type="string"), required=False),
        ParamSpec("reservoir_gate", TypeDescriptor(base_type="string"), required=False),
        ParamSpec("screening_gate", TypeDescriptor(base_type="string"), required=False),
        ParamSpec("extra", TypeDescriptor(base_type="string"), required=False),
        ParamSpec("uncertainty", TypeDescriptor(base_type="float"), required=False),
        ParamSpec("hash", TypeDescriptor(base_type="string"), required=False),
        ParamSpec("time", TypeDescriptor(base_type="int"), required=False),
        ParamSpec("state", TypeDescriptor(base_type="string"), required=False),
        ParamSpec("unit_name", TypeDescriptor(base_type="string"), required=False),
    ]

    _union_value = TypeDescriptor.make_union([
        TypeDescriptor(base_type="int"),
        TypeDescriptor(base_type="float"),
        TypeDescriptor(base_type="bool"),
        TypeDescriptor(base_type="string"),
    ])

    reg.register(BuiltinSignature(
        name="readLatest",
        params=[
            ParamSpec("scope", TypeDescriptor(base_type="string"), required=True),
            ParamSpec("name", TypeDescriptor(base_type="string"), required=True),
            *_optional_db_params,
        ],
        returns=[
            ParamSpec("characteristic", _union_value),
            ParamSpec("error", TypeDescriptor(base_type="Error")),
        ],
        uses_named_args=True,
    ))

    # --------------------------------------------------------- database: write
    reg.register(BuiltinSignature(
        name="write",
        params=[
            ParamSpec("scope", TypeDescriptor(base_type="string"), required=True),
            ParamSpec("name", TypeDescriptor(base_type="string"), required=True),
            ParamSpec("value", _union_value, required=True),
            *_optional_db_params,
        ],
        returns=[ParamSpec("out", TypeDescriptor(base_type="Error"))],
        uses_named_args=True,
    ))

    # ---------------------------------------------------------------- type members

    # Config members
    reg.register_member("Config", MemberSpec(
        "plunger_gates", TypeDescriptor(base_type="Connections"),
    ))
    reg.register_member("Config", MemberSpec(
        "barrier_gates", TypeDescriptor(base_type="Connections"),
    ))
    reg.register_member("Config", MemberSpec(
        "get_group_plunger_gates",
        TypeDescriptor(base_type="Connections"),
        is_method=True,
        method_params=[ParamSpec("gname", TypeDescriptor(base_type="Gname"), required=True)],
    ))

    # Connection members
    reg.register_member("Connection", MemberSpec(
        "name", TypeDescriptor(base_type="string"), is_method=True,
    ))
    reg.register_member("Connection", MemberSpec(
        "value", TypeDescriptor(base_type="float"), is_method=True,
    ))

    # Connections members
    reg.register_member("Connections", MemberSpec(
        "size", TypeDescriptor(base_type="int"), is_method=True,
    ))

    # Quantity members
    reg.register_member("Quantity", MemberSpec(
        "value", TypeDescriptor(base_type="float"),
    ))
    reg.register_member("Quantity", MemberSpec(
        "unit", TypeDescriptor(base_type="string"),
    ))

    return reg


# Module-level singleton
DEFAULT_REGISTRY: BuiltinRegistry = create_default_registry()
