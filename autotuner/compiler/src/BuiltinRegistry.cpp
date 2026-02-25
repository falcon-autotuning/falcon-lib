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
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Void))},
      true));

  registry.register_builtin(BuiltinSignature(
      "logWarn",
      {BuiltinSignature::ParamSpec("format", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Void))},
      true));

  registry.register_builtin(BuiltinSignature(
      "logError",
      {BuiltinSignature::ParamSpec("format", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Void))},
      true));

  // ========================================================================
  // ERROR CONSTRUCTION
  // ========================================================================

  registry.register_builtin(BuiltinSignature(
      "errorMsg",
      {BuiltinSignature::ParamSpec("message", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Error))},
      true));

  registry.register_builtin(BuiltinSignature(
      "fatalErrorMsg",
      {BuiltinSignature::ParamSpec("message", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Error))},
      true));

  // ========================================================================
  // DATABASE FUNCTIONS
  // ========================================================================

  // read(scope, name) -> (union<int|float|bool|string|nil>, Error)
  registry.register_builtin(BuiltinSignature(
      "read",
      {BuiltinSignature::ParamSpec("scope", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec("name", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec(
           "characteristic",
           TypeDescriptor::make_union({TypeDescriptor(ParamType::Int),
                                       TypeDescriptor(ParamType::Float),
                                       TypeDescriptor(ParamType::Bool),
                                       TypeDescriptor(ParamType::String)})),
       BuiltinSignature::ParamSpec("error", TypeDescriptor(ParamType::Error))},
      true));

  // write(scope, name, value) -> Error
  registry.register_builtin(BuiltinSignature(
      "write",
      {BuiltinSignature::ParamSpec("scope", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec("name", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec(
           "value",
           TypeDescriptor::make_union({TypeDescriptor(ParamType::Int),
                                       TypeDescriptor(ParamType::Float),
                                       TypeDescriptor(ParamType::Bool),
                                       TypeDescriptor(ParamType::String)}),
           true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Error))},
      true));

  return registry;
}

} // namespace falcon::atc
