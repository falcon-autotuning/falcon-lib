#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace falcon::autotuner {

// Method callable: receives the receiver object and a param map,
// returns a FunctionResult.
using TypeMethod =
    std::function<FunctionResult(const RuntimeValue &, const ParameterMap &)>;

class TypeRegistry {
public:
  static std::shared_ptr<TypeRegistry> create_default();

  // FAL struct declarations (parsed from .fal files)
  void register_struct(const atc::StructDecl *decl);
  [[nodiscard]] const atc::StructDecl *
  lookup_struct(const std::string &name) const;

  // FFI method dispatch (registered by AutotunerEngine when loading ffimports)
  void register_ffi_method(const std::string &type_name,
                           const std::string &method_name, TypeMethod method);
  [[nodiscard]] const TypeMethod *
  lookup_method(const std::string &type_name,
                const std::string &method_name) const;

private:
  // Keyed by struct name → StructDecl*
  std::map<std::string, const atc::StructDecl *> struct_registry_;

  // Keyed by type_name → method_name → callable
  std::map<std::string, std::map<std::string, TypeMethod>> method_registry_;
};

} // namespace falcon::autotuner
