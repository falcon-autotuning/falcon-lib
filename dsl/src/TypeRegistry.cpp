#include "falcon-dsl/TypeRegistry.hpp"
#include <falcon-typing/PrimitiveTypes.hpp>
#include <stdexcept>
#include <string>

namespace falcon::dsl {

std::shared_ptr<TypeRegistry> TypeRegistry::create_default() {
  auto registry = std::make_shared<TypeRegistry>();

  // ── Array built-in methods ─────────────────────────────────────────────────

  // erase(int index) — removes the element at the given index in-place.
  // Returns nothing (empty FunctionResult); mutates via shared_ptr.
  registry->register_method(
      "Array", "erase",
      [](const typing::RuntimeValue &obj,
         const typing::ParameterMap &params) -> typing::FunctionResult {
        const auto &arrPtr = std::get<std::shared_ptr<typing::ArrayValue>>(obj);
        if (!arrPtr) {
          throw std::runtime_error("erase: called on nil Array");
        }
        auto it = params.find("index");
        if (it == params.end()) {
          throw std::runtime_error("erase: missing argument 'index'");
        }
        int64_t idx = std::get<int64_t>(it->second);
        if (idx < 0 || static_cast<size_t>(idx) >= arrPtr->elements.size()) {
          throw std::runtime_error(
              "erase: index out of bounds: " + std::to_string(idx) +
              " (size=" + std::to_string(arrPtr->elements.size()) + ")");
        }
        arrPtr->elements.erase(arrPtr->elements.begin() + idx);
        return {};
      });

  // insert(int index, value) — inserts value before the element at index.
  // Returns nothing (empty FunctionResult); mutates via shared_ptr.
  registry->register_method(
      "Array", "insert",
      [](const typing::RuntimeValue &obj,
         const typing::ParameterMap &params) -> typing::FunctionResult {
        const auto &arrPtr = std::get<std::shared_ptr<typing::ArrayValue>>(obj);
        if (!arrPtr) {
          throw std::runtime_error("insert: called on nil Array");
        }
        auto idx_it = params.find("index");
        auto val_it = params.find("value");
        if (idx_it == params.end()) {
          throw std::runtime_error("insert: missing argument 'index'");
        }
        if (val_it == params.end()) {
          throw std::runtime_error("insert: missing argument 'value'");
        }
        int64_t idx = std::get<int64_t>(idx_it->second);
        if (idx < 0 || static_cast<size_t>(idx) > arrPtr->elements.size()) {
          throw std::runtime_error(
              "insert: index out of bounds: " + std::to_string(idx) +
              " (size=" + std::to_string(arrPtr->elements.size()) + ")");
        }
        arrPtr->elements.insert(arrPtr->elements.begin() + idx, val_it->second);
        return {};
      });

  // pushback(value) — appends value to the end of the array.
  // Returns nothing (empty FunctionResult); mutates via shared_ptr.
  registry->register_method(
      "Array", "pushback",
      [](const typing::RuntimeValue &obj,
         const typing::ParameterMap &params) -> typing::FunctionResult {
        const auto &arrPtr = std::get<std::shared_ptr<typing::ArrayValue>>(obj);
        if (!arrPtr) {
          throw std::runtime_error("pushback: called on nil Array");
        }
        auto val_it = params.find("value");
        if (val_it == params.end()) {
          throw std::runtime_error("pushback: missing argument 'value'");
        }
        arrPtr->elements.push_back(val_it->second);
        return {};
      });

  // size() — returns the number of elements in the array.
  // Returns: { static_cast<int64_t>(elements.size()) }
  registry->register_method(
      "Array", "size",
      [](const typing::RuntimeValue &obj,
         const typing::ParameterMap & /*params*/) -> typing::FunctionResult {
        const auto &arrPtr = std::get<std::shared_ptr<typing::ArrayValue>>(obj);
        if (!arrPtr) {
          throw std::runtime_error("size: called on nil Array");
        }
        return {static_cast<int64_t>(arrPtr->elements.size())};
      });

  return registry;
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
  // If not found, check for generic type (e.g., Array[int] -> Array)
  auto lbracket = type_name.find('[');
  if (lbracket != std::string::npos) {
    std::string generic_type = type_name.substr(0, lbracket);
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
