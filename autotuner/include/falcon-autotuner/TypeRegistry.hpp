#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace falcon::autotuner {

class TypeRegistry {
public:
  /**
   * @brief Method signature for BUILTIN instance methods (Connection,
   * Quantity…). Returns ParameterMap — legacy interface kept for backward
   * compatibility.
   */
  using MethodFunction = std::function<ParameterMap(
      const RuntimeValue &obj, const ParameterMap &params)>;

  // ── Builtin (non-FFI) methods ─────────────────────────────────────────────
  void register_method(const std::string &type_name,
                       const std::string &method_name, MethodFunction func);
  MethodFunction *lookup_method(const std::string &type_name,
                                const std::string &method_name);
  [[nodiscard]] bool has_method(const std::string &type_name,
                                const std::string &method_name) const;

  // ── FFI struct methods ────────────────────────────────────────────────────
  /**
   * @brief Register a TypeMethod for a user-defined FFI struct.
   *
   * These are called by AutotunerEngine::process_ff_import when it loads a
   * shared library and binds STRUCT<Type><Method> symbols.
   *
   * TypeMethod returns FunctionResult (ordered output list) rather than
   * ParameterMap, matching the rest of the struct routine dispatch.
   */
  void register_ffi_method(const std::string &type_name,
                           const std::string &method_name, TypeMethod method);

  /**
   * @brief Look up a registered FFI TypeMethod.
   * Returns nullptr if no FFI method is registered for this type+method pair.
   */
  [[nodiscard]] const TypeMethod *
  lookup_ffi_method(const std::string &type_name,
                    const std::string &method_name) const;

  // ── Struct declarations ───────────────────────────────────────────────────
  void register_struct(const atc::StructDecl *decl) {
    // Register bare name
    struct_registry_[decl->name] = decl;
    // Register qualified name if module is known
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
  // Builtin methods: type_name → method_name → MethodFunction (→ ParameterMap)
  std::map<std::string, std::map<std::string, MethodFunction>> type_methods_;

  // FFI struct methods: type_name → method_name → TypeMethod (→ FunctionResult)
  std::map<std::string, std::map<std::string, TypeMethod>> ffi_methods_;

  std::map<std::string, const atc::StructDecl *> struct_registry_;
};

void register_all_type_methods(TypeRegistry &registry);

} // namespace falcon::autotuner
