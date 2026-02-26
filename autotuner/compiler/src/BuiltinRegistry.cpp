#include "falcon-atc/AST.hpp"

namespace falcon::atc {

BuiltinFunctionRegistry BuiltinFunctionRegistry::create_default() {
  BuiltinFunctionRegistry registry;

  // ========================================================================
  // LOGGING FUNCTIONS
  // ========================================================================

  registry.register_builtin(BuiltinSignature(
      "logInfo",
      {BuiltinSignature::ParamSpec("format", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Void))}));

  registry.register_builtin(BuiltinSignature(
      "logWarn",
      {BuiltinSignature::ParamSpec("format", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Void))}));

  registry.register_builtin(BuiltinSignature(
      "logError",
      {BuiltinSignature::ParamSpec("format", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Void))}));

  // ========================================================================
  // ERROR CONSTRUCTION
  // ========================================================================

  registry.register_builtin(BuiltinSignature(
      "errorMsg",
      {BuiltinSignature::ParamSpec("message", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Error))}));

  registry.register_builtin(BuiltinSignature(
      "fatalErrorMsg",
      {BuiltinSignature::ParamSpec("message", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Error))}));

  // ========================================================================
  // DATABASE FUNCTIONS
  // ========================================================================

  // readLatest(scope, name, ...) -> (union<int|float|bool|string|nil>, Error)
  registry.register_builtin(BuiltinSignature(
      "readLatest",
      {BuiltinSignature::ParamSpec("scope", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec("name", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec(
           "barrier_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec(
           "plunger_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec(
           "reservoir_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec(
           "screening_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec("extra", TypeDescriptor(ParamType::String),
                                   false),
       BuiltinSignature::ParamSpec("uncertainty",
                                   TypeDescriptor(ParamType::Float), false),
       BuiltinSignature::ParamSpec("hash", TypeDescriptor(ParamType::String),
                                   false),
       BuiltinSignature::ParamSpec("time", TypeDescriptor(ParamType::Int),
                                   false),
       BuiltinSignature::ParamSpec("state", TypeDescriptor(ParamType::String),
                                   false),
       BuiltinSignature::ParamSpec("unit_name",
                                   TypeDescriptor(ParamType::String), false)},
      {BuiltinSignature::ParamSpec(
           "characteristic",
           TypeDescriptor::make_union({TypeDescriptor(ParamType::Int),
                                       TypeDescriptor(ParamType::Float),
                                       TypeDescriptor(ParamType::Bool),
                                       TypeDescriptor(ParamType::String)})),
       BuiltinSignature::ParamSpec("error", TypeDescriptor(ParamType::Error))},
      NamedArgs::Uses));

  // write(scope, name, value, ...) -> Error
  registry.register_builtin(BuiltinSignature(
      "write",
      {BuiltinSignature::ParamSpec("scope", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec("name", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec(
           "characteristic",
           TypeDescriptor::make_union({TypeDescriptor(ParamType::Int),
                                       TypeDescriptor(ParamType::Float),
                                       TypeDescriptor(ParamType::Bool),
                                       TypeDescriptor(ParamType::String)}),
           true),
       BuiltinSignature::ParamSpec(
           "barrier_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec(
           "plunger_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec(
           "reservoir_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec(
           "screening_gate", TypeDescriptor(ParamType::Connection), false),
       BuiltinSignature::ParamSpec("extra", TypeDescriptor(ParamType::String),
                                   false),
       BuiltinSignature::ParamSpec("uncertainty",
                                   TypeDescriptor(ParamType::Float), false),
       BuiltinSignature::ParamSpec("hash", TypeDescriptor(ParamType::String),
                                   false),
       BuiltinSignature::ParamSpec("time", TypeDescriptor(ParamType::Int),
                                   false),
       BuiltinSignature::ParamSpec("state", TypeDescriptor(ParamType::String),
                                   false),
       BuiltinSignature::ParamSpec("unit_name",
                                   TypeDescriptor(ParamType::String), false)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Error))},
      NamedArgs::Uses));

  return registry;
}

} // namespace falcon::atc
