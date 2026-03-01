#include "falcon-autotuner/TypeRegistry.hpp"

namespace falcon::autotuner {

std::shared_ptr<TypeRegistry> TypeRegistry::create_default() {
  auto registry = std::make_shared<TypeRegistry>();
  // No hard-coded falcon_core types. All types are registered dynamically
  // via register_struct() (FAL structs) or register_ffi_method() (FFI structs).
  return registry;
}

void TypeRegistry::register_struct(const atc::StructDecl *decl) {
  if (decl) {
    struct_registry_[decl->name] = decl;
  }
}

const atc::StructDecl *
TypeRegistry::lookup_struct(const std::string &name) const {
  auto it = struct_registry_.find(name);
  return it != struct_registry_.end() ? it->second : nullptr;
}

void TypeRegistry::register_ffi_method(const std::string &type_name,
                                       const std::string &method_name,
                                       TypeMethod method) {
  method_registry_[type_name][method_name] = std::move(method);
}

const TypeMethod *
TypeRegistry::lookup_method(const std::string &type_name,
                            const std::string &method_name) const {
  auto type_it = method_registry_.find(type_name);
  if (type_it == method_registry_.end())
    return nullptr;
  auto method_it = type_it->second.find(method_name);
  if (method_it == type_it->second.end())
    return nullptr;
  return &method_it->second;
}

} // namespace falcon::autotuner
