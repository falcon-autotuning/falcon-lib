#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <stdexcept>
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
  std::string type_name;

  std::shared_ptr<std::map<std::string, RuntimeValue>> fields =
      std::make_shared<std::map<std::string, RuntimeValue>>();

  // ── Native (FFI) handle ─────────────────────────────────────────────────
  // When a struct is backed by a C++ object (e.g. a `shared_ptr<Quantity>`
  // allocated in a wrapper), native_handle holds a heap-allocated
  // `shared_ptr<T>*` and native_deleter frees it.
  // is_native() returns true iff a native handle is present.
  void *native_handle = nullptr;
  void (*native_deleter)(void *) = nullptr;
  const char *native_type_name = nullptr; // e.g. "Quantity", "Connection"

  StructInstance() = default;
  explicit StructInstance(std::string typeName)
      : type_name(std::move(typeName)) {}

  ~StructInstance() {
    if (native_handle && native_deleter) {
      native_deleter(native_handle);
      native_handle = nullptr;
    }
  }

  // Non-copyable because of raw pointer ownership:
  StructInstance(const StructInstance &) = delete;
  StructInstance &operator=(const StructInstance &) = delete;
  StructInstance(StructInstance &&) = default;
  StructInstance &operator=(StructInstance &&) = default;

  [[nodiscard]] bool is_native() const { return native_handle != nullptr; }

  // Set a native handle (takes ownership; previous handle is freed).
  template <typename T>
  void set_native(std::shared_ptr<T> sp, const char *type_name_str) {
    if (native_handle && native_deleter)
      native_deleter(native_handle);
    native_handle = new std::shared_ptr<T>(std::move(sp));
    native_type_name = type_name_str;
    native_deleter = [](void *p) {
      delete static_cast<std::shared_ptr<T> *>(p);
    };
  }

  // Retrieve the native handle as shared_ptr<T>. Throws if wrong type.
  template <typename T> [[nodiscard]] std::shared_ptr<T> get_native() const {
    if (!native_handle)
      throw std::runtime_error("StructInstance '" + type_name +
                               "' has no native handle");
    return *static_cast<std::shared_ptr<T> *>(native_handle);
  }

  [[nodiscard]] RuntimeValue &get_field(const std::string &fieldName) {
    return (*fields)[fieldName];
  }
  [[nodiscard]] const RuntimeValue &
  get_field(const std::string &fieldName) const {
    auto it = fields->find(fieldName);
    if (it == fields->end())
      throw std::runtime_error("Struct '" + type_name +
                               "' has no field: " + fieldName);
    return it->second;
  }
  void set_field(const std::string &fieldName, RuntimeValue val) {
    (*fields)[fieldName] = std::move(val);
  }
  bool operator==(const StructInstance &other) const {
    return type_name == other.type_name && *fields == *other.fields;
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
