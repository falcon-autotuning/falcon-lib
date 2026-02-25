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

struct ErrorObject {
  std::string message;
  bool is_fatal = false;

  ErrorObject() = default;
  ErrorObject(std::string msg, bool fatal = false)
      : message(std::move(msg)), is_fatal(fatal) {}

  bool operator==(const ErrorObject &other) const {
    return this->message == other.message && this->is_fatal == other.is_fatal;
  }
  bool operator!=(const ErrorObject &other) const { return !(*this == other); }
};

// Forward declaration
struct TupleValue;
/**
 * @brief Runtime value that can hold any parameter value.
 *
 * This includes:
 * - Primitive types (int, float, bool, string)
 * - Falcon-core object wrappers (Connection, Quantity, etc.)
 * - Special values (nil, Error)
 */
using RuntimeValue =
    std::variant<int64_t, double, bool, std::string,
                 std::nullptr_t, // nil
                 falcon_core::physics::device_structures::ConnectionSP,
                 falcon_core::physics::device_structures::ConnectionsSP,
                 falcon_core::math::QuantitySP,
                 falcon_core::autotuner_interfaces::names::GnameSP, ErrorObject,
                 TupleValue>;

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

  explicit TupleValue(std::vector<RuntimeValue> vals)
      : values(std::move(vals)) {}

  bool operator==(const TupleValue &other) const {
    return values == other.values;
  }
  bool operator!=(const TupleValue &other) const { return !(*this == other); }
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
