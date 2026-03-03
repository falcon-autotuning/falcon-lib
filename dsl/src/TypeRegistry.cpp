#include "falcon-dsl/TypeRegistry.hpp"

namespace falcon::dsl {

std::shared_ptr<TypeRegistry> TypeRegistry::create_default() {
  auto registry = std::make_shared<TypeRegistry>();
  // No hard-coded falcon_core types. All types are registered dynamically
  // via register_struct() (FAL structs) or register_ffi_method() (FFI structs).
  return registry;
}

void TypeRegistry::register_ffi_method(const std::string &type_name,
                                       const std::string &method_name,
                                       typing::TypeMethod method) {
  ffi_methods_[type_name][method_name] = std::move(method);
}

TypeRegistry::MethodFunction *
TypeRegistry::lookup_method(const std::string &type_name,
                            const std::string &method_name) {
  auto type_it = type_methods_.find(type_name);
  if (type_it == type_methods_.end()) {
    return nullptr;
  }
  auto method_it = type_it->second.find(method_name);
  if (method_it == type_it->second.end()) {
    return nullptr;
  }
  return &method_it->second;
}

const typing::TypeMethod *
TypeRegistry::lookup_ffi_method(const std::string &type_name,
                                const std::string &method_name) const {
  auto type_it = ffi_methods_.find(type_name);
  if (type_it == ffi_methods_.end()) {
    return nullptr;
  }
  auto method_it = type_it->second.find(method_name);
  if (method_it == type_it->second.end()) {
    return nullptr;
  }
  return &method_it->second;
}

} // namespace falcon::dsl
