#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace falcon::autotuner {

/**
 * @brief Registry for type information and method bindings.
 *
 * Each type (Connection, Connections, Quantity, etc.) has:
 * - Instance methods that can be called on objects
 * - These are registered once at startup
 */
class TypeRegistry {
public:
  /**
   * @brief Method signature for instance methods.
   * Takes the object ('this') plus named parameters.
   */
  using MethodFunction = std::function<ParameterMap(
      const RuntimeValue &obj, const ParameterMap &params)>;

  /**
   * @brief Register a method for a type.
   *
   * @param type_name Type name (e.g., "Connection")
   * @param method_name Method name (e.g., "name")
   * @param func Implementation
   */
  void register_method(const std::string &type_name,
                       const std::string &method_name, MethodFunction func);

  /**
   * @brief Look up a method for a type.
   *
   * @param type_name Type name
   * @param method_name Method name
   * @return Method function or nullptr if not found
   */
  MethodFunction *lookup_method(const std::string &type_name,
                                const std::string &method_name);

  /**
   * @brief Check if a method exists for a type.
   */
  [[nodiscard]] bool has_method(const std::string &type_name,
                                const std::string &method_name) const;

  /**
   * @brief Create default type registry with all builtin types.
   */
  static std::shared_ptr<TypeRegistry> create_default();
  // Register a struct declaration so the interpreter can look it up by type
  // name.
  void register_struct(const atc::StructDecl *decl) {
    struct_registry_[decl->name] = decl;
  }

  // Look up a struct declaration by type name (e.g. "Quantity").
  // Returns nullptr if not registered.
  [[nodiscard]] const atc::StructDecl *
  lookup_struct(const std::string &type_name) const {
    auto it = struct_registry_.find(type_name);
    return it != struct_registry_.end() ? it->second : nullptr;
  }

private:
  // Map: type_name -> (method_name -> method_impl)
  std::map<std::string, std::map<std::string, MethodFunction>> type_methods_;
  std::map<std::string, const atc::StructDecl *> struct_registry_;
};

/**
 * @brief Register all builtin type methods (called internally).
 */
void register_all_type_methods(TypeRegistry &registry);

} // namespace falcon::autotuner
