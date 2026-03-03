#pragma once

#include "falcon-atc/AST.hpp"
#include <falcon-typing/PrimitiveTypes.hpp>
#include <map>
#include <memory>
#include <string>

namespace falcon::dsl {

class TypeRegistry {
public:
  // ── Built-in (non-FFI) default type methods ───────────────────────────────
  //
  // Both built-in methods (Array, etc.) and FFI struct methods use the same
  // TypeMethod signature:
  //
  //   FunctionResult fn(const RuntimeValue& obj, const ParameterMap& params)
  //
  // Built-in methods are registered here via create_default().
  // FFI struct methods are registered at runtime when a shared library is
  // loaded by AutotunerEngine::process_ff_import.

  void register_method(const std::string &type_name,
                       const std::string &method_name,
                       typing::TypeMethod method);

  [[nodiscard]] const typing::TypeMethod *
  lookup_method(const std::string &type_name,
                const std::string &method_name) const;

  // ── FFI struct methods ────────────────────────────────────────────────────
  void register_ffi_method(const std::string &type_name,
                           const std::string &method_name,
                           typing::TypeMethod method);

  [[nodiscard]] const typing::TypeMethod *
  lookup_ffi_method(const std::string &type_name,
                    const std::string &method_name) const;

  // ── Struct declarations ───────────────────────────────────────────────────
  void register_struct(const atc::StructDecl *decl) {
    struct_registry_[decl->name] = decl;
    if (!decl->module_name.empty()) {
      struct_registry_[decl->module_name + "::" + decl->name] = decl;
    }
  }

  [[nodiscard]] const atc::StructDecl *
  lookup_struct(const std::string &type_name) const {
    auto it = struct_registry_.find(type_name);
    return it != struct_registry_.end() ? it->second : nullptr;
  }

  static std::shared_ptr<TypeRegistry> create_default();

private:
  // Built-in methods for default types (Array, etc.)
  std::map<std::string, std::map<std::string, typing::TypeMethod>>
      type_methods_;

  // FFI methods for user-defined structs loaded from shared libraries
  std::map<std::string, std::map<std::string, typing::TypeMethod>> ffi_methods_;

  std::map<std::string, const atc::StructDecl *> struct_registry_;
};

void register_all_type_methods(TypeRegistry &registry);

} // namespace falcon::dsl
