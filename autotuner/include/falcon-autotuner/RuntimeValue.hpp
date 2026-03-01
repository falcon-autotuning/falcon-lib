#pragma once

#include <cstdint>
#include <falcon_core/autotuner_interfaces/names/Gname.hpp>
#include <falcon_core/math/Quantity.hpp>
#include <falcon_core/physics/device_structures/Connection.hpp>
#include <falcon_core/physics/device_structures/Connections.hpp>
#include <map>
#include <memory>
#include <string>
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
 */
using RuntimeValue =
    std::variant<int64_t, double, bool, std::string,
                 std::nullptr_t, // nil
                 falcon_core::physics::device_structures::ConnectionSP,
                 falcon_core::physics::device_structures::ConnectionsSP,
                 falcon_core::math::QuantitySP,
                 falcon_core::autotuner_interfaces::names::GnameSP, ErrorObject,
                 std::shared_ptr<TupleValue>,
                 std::shared_ptr<StructInstance>>; // User-defined struct
                                                   // instances

/**
 * @brief Wrapper for tuple values (multiple return values).
 *
 * This is a temporary container used during tuple destructuring:
 *   a, b = read(scope="x", name="y")
 *
 * The TupleValue holds the ordered values until they're assigned.
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
 * @brief A live instance of a user-defined struct type.
 *
 * Created when a struct constructor routine (e.g. New, NewWithB) is called.
 * Fields are stored in a map keyed by field name.
 * The type_name identifies which StructDecl this instance belongs to,
 * allowing the interpreter to dispatch method calls and operator overloads.
 */
struct StructInstance {
  std::string type_name; // e.g. "Quantity"

  // Field storage — initialized from StructDecl defaults, then set by routines
  // Uses shared_ptr so StructInstance can be cheaply copied into RuntimeValue
  std::shared_ptr<std::map<std::string, RuntimeValue>> fields =
      std::make_shared<std::map<std::string, RuntimeValue>>();

  // Native handle for FFI structs: holds the underlying C++ object (e.g.
  // shared_ptr<Connection>).  std::nullopt means this is a pure-FAL struct.
  // Stored as RuntimeValue so it can hold any of the known shared_ptr types
  // already in the variant without adding shared_ptr<void>.
  std::optional<RuntimeValue> native_handle = std::nullopt;

  StructInstance() = default;
  explicit StructInstance(std::string typeName)
      : type_name(std::move(typeName)) {}

  [[nodiscard]] RuntimeValue &get_field(const std::string &fieldName) {
    return (*fields)[fieldName];
  }
  [[nodiscard]] const RuntimeValue &
  get_field(const std::string &fieldName) const {
    return fields->at(fieldName);
  }
  void set_field(const std::string &fieldName, RuntimeValue val) {
    (*fields)[fieldName] = std::move(val);
  }
  [[nodiscard]] bool has_field(const std::string &fieldName) const {
    return fields->contains(fieldName);
  }
};

/**
 * @brief Parameter map for passing values to/from functions.
 */
using ParameterMap = std::map<std::string, RuntimeValue>;
using FunctionResult = std::vector<RuntimeValue>;

/**
 * @brief Helper to get type name as string.
 */
std::string get_runtime_type_name(const RuntimeValue &value);

/**
 * @brief Helper to print runtime value (for debugging).
 */
std::string runtime_value_to_string(const RuntimeValue &value);

} // namespace falcon::autotuner
