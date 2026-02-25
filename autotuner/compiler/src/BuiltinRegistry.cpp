#include "falcon-atc/AST.hpp"

namespace falcon::atc {

BuiltinFunctionRegistry BuiltinFunctionRegistry::create_default() {
  BuiltinFunctionRegistry registry;

  // Register log::* functions
  registry.register_builtin(
      BuiltinSignature("logInfo",
                       {BuiltinSignature::ParamSpec(
                           "format", TypeDescriptor(ParamType::String), true)},
                       TypeDescriptor(ParamType::Nil), false));

  registry.register_builtin(
      BuiltinSignature("logWarn",
                       {BuiltinSignature::ParamSpec(
                           "format", TypeDescriptor(ParamType::String), true)},
                       TypeDescriptor(ParamType::Nil), false));

  registry.register_builtin(
      BuiltinSignature("logError",
                       {BuiltinSignature::ParamSpec(
                           "format", TypeDescriptor(ParamType::String), true)},
                       TypeDescriptor(ParamType::Nil), false));

  // Register Error::* functions
  registry.register_builtin(
      BuiltinSignature("errorMsg",
                       {BuiltinSignature::ParamSpec(
                           "message", TypeDescriptor(ParamType::String), true)},
                       TypeDescriptor(ParamType::Error), false));

  registry.register_builtin(
      BuiltinSignature("fatalErrorMsg",
                       {BuiltinSignature::ParamSpec(
                           "message", TypeDescriptor(ParamType::String), true)},
                       TypeDescriptor(ParamType::Error), false));

  // Register database functions
  registry.register_builtin(BuiltinSignature(
      "read",
      {BuiltinSignature::ParamSpec("scope", TypeDescriptor(ParamType::String),
                                   true),
       BuiltinSignature::ParamSpec("name", TypeDescriptor(ParamType::String),
                                   true)},
      TypeDescriptor::make_union(
          {TypeDescriptor(ParamType::Int), TypeDescriptor(ParamType::Float),
           TypeDescriptor(ParamType::Bool), TypeDescriptor(ParamType::String)}),
      true));

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
           true)},
      TypeDescriptor(ParamType::Nil), false));

  return registry;
}

} // namespace falcon::atc
