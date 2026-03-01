#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace falcon::autotuner {

// Forward declarations
struct TupleValue;
struct StructInstance;

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
 * Note: Uses shared_ptr for TupleValue and StructInstance to break circular
 * dependency.
 *
 * All previously hard-coded falcon_core types (Connection, Quantity, etc.)
 * have been removed. They are now bound through StructInstance::native_handle.
 */
using RuntimeValue =
    std::variant<int64_t, double, bool, std::string,
                 std::nullptr_t,                 // nil
                 ErrorObject,                    // error
                 std::shared_ptr<TupleValue>,    // tuple/multi-return
                 std::shared_ptr<StructInstance> // user-defined or FFI struct
                 >;

/**
 * @brief Wrapper for tuple values (multiple return values).
 */
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
 * @brief A live instance of a user-defined struct type, or an FFI-bound
 * C++ object.
 *
 * For pure-FAL structs: fields holds all field values, native_handle is empty.
 *
 * For FFI-bound structs (e.g. a C++ Quantity wrapping falcon_core):
 *   - native_handle holds a shared_ptr<void> that owns the real C++ object.
 *   - fields may be empty — the FFI dispatch functions receive the
 *     native_handle as params["this"] and cast it back to the concrete type.
 *   - type_name still identifies which struct type this is, so method
 *     dispatch still works correctly.
 *
 * The native_handle is typed as shared_ptr<void> for type erasure. The
 * wrapper code (e.g. QuantityW.cpp) casts it back via:
 *   auto q = std::static_pointer_cast<Quantity>(
 *       std::get<std::shared_ptr<StructInstance>>(params.at("this"))
 *           ->native_handle.value());
 */
struct StructInstance {
  std::string type_name; // e.g. "Quantity"

  // Field storage — for pure-FAL structs. FFI structs may leave this empty.
  std::shared_ptr<std::map<std::string, RuntimeValue>> fields =
      std::make_shared<std::map<std::string, RuntimeValue>>();

  // Type-erased handle to the underlying C++ object for FFI structs.
  // Empty for pure-FAL structs.
  std::optional<std::shared_ptr<void>> native_handle;

  StructInstance() = default;
  explicit StructInstance(std::string typeName)
      : type_name(std::move(typeName)) {}

  /// Convenience: construct from a concrete C++ shared_ptr.
  /// Usage:  StructInstance::from_native("Quantity", q_ptr)
  template <typename T>
  static std::shared_ptr<StructInstance> from_native(std::string type_name,
                                                     std::shared_ptr<T> ptr) {
    auto inst = std::make_shared<StructInstance>(std::move(type_name));
    inst->native_handle = std::static_pointer_cast<void>(ptr);
    return inst;
  }

  /// Convenience: retrieve the native pointer cast to a concrete type.
  /// Throws std::bad_cast (via std::bad_optional_access) if no handle.
  template <typename T> std::shared_ptr<T> get_native() const {
    return std::static_pointer_cast<T>(native_handle.value());
  }

  /// Returns true if this instance is backed by a native C++ object.
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
    (*fields)[fieldName] = std::move(val);
  }

  bool operator==(const StructInstance &other) const {
    return type_name == other.type_name && *fields == *other.fields &&
           native_handle == other.native_handle;
  }
  bool operator!=(const StructInstance &other) const {
    return !(*this == other);
  }
};

/**
 * @brief Parameter map for passing values to/from functions.
 */
using ParameterMap = std::map<std::string, RuntimeValue>;
using FunctionResult = std::vector<RuntimeValue>;

/**
 * @brief The type signature for an externally-loaded (FFI) function symbol.
 *
 * The "this" parameter for instance methods is passed as params["this"]
 * containing a shared_ptr<StructInstance> whose native_handle holds the
 * real C++ object. Constructor functions do NOT receive "this".
 */
using FFIFunction = FunctionResult (*)(ParameterMap);

/**
 * @brief Helper to get type name as string.
 */
std::string get_runtime_type_name(const RuntimeValue &value);

/**
 * @brief Helper to print runtime value (for debugging).
 */
std::string runtime_value_to_string(const RuntimeValue &value);

} // namespace falcon::autotuner
