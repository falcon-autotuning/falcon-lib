#include "falcon-dsl/TypeRegistry.hpp"
#include <falcon-typing/PrimitiveTypes.hpp>
#include <stdexcept>
#include <string>

namespace falcon::dsl {

std::shared_ptr<TypeRegistry> TypeRegistry::create_default() {
  // The default registry no longer hard-codes any built-in Array methods.
  // Array<T> is now a first-class generic struct defined in
  // libs/collections/array/array.fal with FFI-backed routines in
  // array-wrapper.cpp.  The TypeRegistry for arrays is populated at
  // load-time when the user imports that library (or when falconCore
  // loads its wrappers, which still use ArrayValue directly for internal
  // data transfer).
  //
  // Map<K,V> is similarly defined in libs/collections/map/map.fal.
  return std::make_shared<TypeRegistry>();
}

void TypeRegistry::register_method(const std::string &type_name,
                                   const std::string &method_name,
                                   typing::TypeMethod method) {
  type_methods_[type_name][method_name] = std::move(method);
}

void TypeRegistry::register_ffi_method(const std::string &type_name,
                                       const std::string &method_name,
                                       typing::TypeMethod method) {
  ffi_methods_[type_name][method_name] = std::move(method);
}

const typing::TypeMethod *
TypeRegistry::lookup_method(const std::string &type_name,
                            const std::string &method_name) const {
  // First, try the exact type name
  auto type_it = type_methods_.find(type_name);
  if (type_it != type_methods_.end()) {
    auto method_it = type_it->second.find(method_name);
    if (method_it != type_it->second.end()) {
      return &method_it->second;
    }
  }
  // If not found, check for generic type (e.g., Array[int] -> Array)
  auto lbracket = type_name.find('[');
  if (lbracket != std::string::npos) {
    std::string generic_type = type_name.substr(0, lbracket);
    auto gen_it = type_methods_.find(generic_type);
    if (gen_it != type_methods_.end()) {
      auto method_it = gen_it->second.find(method_name);
      if (method_it != gen_it->second.end()) {
        return &method_it->second;
      }
    }
  }
  return nullptr;
}

const typing::TypeMethod *
TypeRegistry::lookup_ffi_method(const std::string &type_name,
                                const std::string &method_name) const {
  // First, try the exact type name
  auto type_it = ffi_methods_.find(type_name);
  if (type_it != ffi_methods_.end()) {
    auto method_it = type_it->second.find(method_name);
    if (method_it != type_it->second.end()) {
      return &method_it->second;
    }
  }
  // If not found, check for generic type (e.g., Array<int> -> Array)
  auto angle = type_name.find('<');
  if (angle != std::string::npos) {
    std::string generic_type = type_name.substr(0, angle);
    auto gen_it = ffi_methods_.find(generic_type);
    if (gen_it != ffi_methods_.end()) {
      auto method_it = gen_it->second.find(method_name);
      if (method_it != gen_it->second.end()) {
        return &method_it->second;
      }
    }
  }
  return nullptr;
}

} // namespace falcon::dsl
