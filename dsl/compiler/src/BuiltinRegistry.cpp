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

  return registry;
}

} // namespace falcon::atc
