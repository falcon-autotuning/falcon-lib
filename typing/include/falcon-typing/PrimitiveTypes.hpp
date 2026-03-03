#pragma once

#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
namespace falcon::typing {

// Forward declarations
struct TupleValue;
struct StructInstance;
struct ArrayValue;

struct ErrorObject {
  std::string message;
  bool is_fatal = false;

  ErrorObject() = default;
  explicit ErrorObject(std::string msg, bool fatal = false)
      : message(std::move(msg)), is_fatal(fatal) {}

  bool operator==(const ErrorObject &other) const {
    return this->message == other.message && this->is_fatal == other.is_fatal;
  }
  bool operator!=(const ErrorObject &other) const { return !(*this == other); }
};

/**
 * @brief Runtime value that can hold any parameter value.
 */
using RuntimeValue =
    std::variant<int64_t, double, bool, std::string, std::nullptr_t,
                 ErrorObject, std::shared_ptr<TupleValue>,
                 std::shared_ptr<StructInstance>,
                 std::shared_ptr<ArrayValue>>;

struct TupleValue {
  std::vector<RuntimeValue> values;
  TupleValue() = default;
  explicit TupleValue(std::vector<RuntimeValue> vals)
      : values(std::move(vals)) {}
  bool operator==(const TupleValue &other) const {
    return values == other.values;
  }
  bool operator!=(const TupleValue &other) const { return !(*this == other); }
  [[nodiscard]] size_t size() const { return values.size(); }
  RuntimeValue &operator[](size_t idx) { return values[idx]; }
  const RuntimeValue &operator[](size_t idx) const { return values[idx]; }
};

/**
 * @brief A live instance of a user-defined struct type.
 *
 * For pure-FAL structs: fields holds all field values, native_handle is empty.
 *
 * For FFI-bound structs (e.g. a C++ Quantity wrapping falcon_core):
 *   - native_handle holds a shared_ptr<void> that owns the real C++ object.
 *   - fields may be empty — the FFI dispatch functions receive the
 *     native_handle as params["this"] and cast it back to the concrete type.
 *   - type_name still identifies which struct type this is, so method
 *     dispatch still works correctly.
 */
struct StructInstance {
  std::string type_name; // e.g. "Quantity"

  std::shared_ptr<std::map<std::string, RuntimeValue>> fields =
      std::make_shared<std::map<std::string, RuntimeValue>>();

  // For FFI-bound structs: holds the actual C++ object (type-erased).
  // Wrapper code casts it back via static_pointer_cast<ConcreteType>.
  std::optional<std::shared_ptr<void>> native_handle;

  StructInstance() = default;
  explicit StructInstance(std::string typeName)
      : type_name(std::move(typeName)) {}

  // Returns true if this instance wraps a native C++ object via the FFI.
  [[nodiscard]] bool is_native() const { return native_handle.has_value(); }

  [[nodiscard]] RuntimeValue &get_field(const std::string &fieldName) {
    return (*fields)[fieldName];
  }
  [[nodiscard]] const RuntimeValue &
  get_field(const std::string &fieldName) const {
    auto fieldIter = fields->find(fieldName);
    if (fieldIter == fields->end()) {
      throw std::runtime_error("Struct '" + type_name +
                               "' has no field: " + fieldName);
    }
    return fieldIter->second;
  }
  void set_field(const std::string &fieldName, RuntimeValue val) {
    (*fields)[fieldName] = val;
  }
  bool operator==(const StructInstance &other) const {
    return type_name == other.type_name && *fields == *other.fields;
  }
  bool operator!=(const StructInstance &other) const {
    return !(*this == other);
  }
};

using ParameterMap = std::map<std::string, RuntimeValue>;
using FunctionResult = std::vector<RuntimeValue>;

/**
 * @brief A generic, dynamically-typed array of RuntimeValues.
 *
 * Array is the first "generic" type: you can have an Array of int,
 * Array of float, Array of string, etc. The element_type_name field
 * records the declared element type (e.g. "int", "float") for
 * serialization and type-checking purposes.
 */
struct ArrayValue {
  std::string element_type_name; // e.g. "int", "float", "string"
  std::vector<RuntimeValue> elements;

  ArrayValue() = default;
  explicit ArrayValue(std::string elem_type)
      : element_type_name(std::move(elem_type)) {}
  ArrayValue(std::string elem_type, std::vector<RuntimeValue> elems)
      : element_type_name(std::move(elem_type)), elements(std::move(elems)) {}

  bool operator==(const ArrayValue &other) const {
    return element_type_name == other.element_type_name &&
           elements == other.elements;
  }
  bool operator!=(const ArrayValue &other) const { return !(*this == other); }
  [[nodiscard]] size_t size() const { return elements.size(); }
  RuntimeValue &operator[](size_t idx) { return elements[idx]; }
  const RuntimeValue &operator[](size_t idx) const { return elements[idx]; }
};

/**
 * @brief Signature for FFI struct instance methods.
 * Returns FunctionResult (ordered outputs), not ParameterMap.
 */
using TypeMethod =
    std::function<FunctionResult(const RuntimeValue &, const ParameterMap &)>;

using FFIFunction = FunctionResult (*)(ParameterMap);

std::string get_runtime_type_name(const RuntimeValue &value);
std::string runtime_value_to_string(const RuntimeValue &value);

} // namespace falcon::typing
