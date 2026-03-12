#include "falcon-atc/AST.hpp"

namespace falcon::atc {

BuiltinFunctionRegistry BuiltinFunctionRegistry::create_default() {
  BuiltinFunctionRegistry registry;

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
  // TYPE COERCION
  // ========================================================================

  registry.register_builtin(BuiltinSignature(
      "int",
      {BuiltinSignature::ParamSpec("val", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Int))}));

  registry.register_builtin(BuiltinSignature(
      "float",
      {BuiltinSignature::ParamSpec("val", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Float))}));

  registry.register_builtin(BuiltinSignature(
      "bool",
      {BuiltinSignature::ParamSpec("val", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::Bool))}));

  registry.register_builtin(BuiltinSignature(
      "string",
      {BuiltinSignature::ParamSpec("val", TypeDescriptor(ParamType::String),
                                   true)},
      {BuiltinSignature::ParamSpec("out", TypeDescriptor(ParamType::String))}));

  return registry;
}

} // namespace falcon::atc
