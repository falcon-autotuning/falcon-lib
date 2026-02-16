#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <utility>

namespace falcon::database {

using json = nlohmann::json;

/**
 * @brief Represents a primitive JSON value that can be stored in the database
 *
 * Supports: null, bool, int64, double, string
 */
class JSONPrimitive {
public:
  enum class Type : std::uint8_t { Null, Boolean, Integer, Double, String };

  JSONPrimitive() : type_(Type::Null) {}
  explicit JSONPrimitive(bool value)
      : type_(Type::Boolean), bool_value_(value) {}
  explicit JSONPrimitive(int64_t value)
      : type_(Type::Integer), int_value_(value) {}
  explicit JSONPrimitive(int value)
      : type_(Type::Integer), int_value_(value) {} // Add int overload
  explicit JSONPrimitive(double value)
      : type_(Type::Double), double_value_(value) {}
  explicit JSONPrimitive(float value)
      : type_(Type::Double), double_value_(value) {} // Add float overload
  explicit JSONPrimitive(std::string value)
      : type_(Type::String), string_value_(std::move(value)) {}
  explicit JSONPrimitive(const char *value)
      : type_(Type::String), string_value_(value) {}

  [[nodiscard]] Type type() const { return type_; }
  [[nodiscard]] bool is_null() const { return type_ == Type::Null; }

  [[nodiscard]] bool as_bool() const;
  [[nodiscard]] int64_t as_int() const;
  [[nodiscard]] double as_double() const;
  [[nodiscard]] std::string as_string() const;

  [[nodiscard]] json to_json() const;
  static JSONPrimitive from_json(const json &json);

private:
  Type type_;
  bool bool_value_ = false;
  int64_t int_value_ = 0;
  double double_value_ = 0.0;
  std::string string_value_;
};

/**
 * @brief Represents a possible query for a device characteristic which is
 * stored in the globals database
 */
struct DeviceCharacteristicQuery {
  std::optional<std::string> scope;
  std::optional<std::string> name;
  std::optional<std::string> barrier_gate;
  std::optional<std::string> plunger_gate;
  std::optional<std::string> reservoir_gate;
  std::optional<std::string> screening_gate;
  std::optional<std::string> extra;
  std::optional<double> uncertainty;
  std::optional<std::string> hash;
  std::optional<int64_t> time;
  std::optional<std::string> state;
  std::optional<std::string> unit_name;
  // No characteristic field here
};
/**
 * @brief Represents a device characteristic which is stored in the globals
 * database
 */
struct DeviceCharacteristic {
  std::string scope;
  std::string name;
  std::optional<std::string> barrier_gate;
  std::optional<std::string> plunger_gate;
  std::optional<std::string> reservoir_gate;
  std::optional<std::string> screening_gate;
  std::optional<std::string> extra;
  std::optional<double> uncertainty;
  std::optional<std::string> hash;
  std::optional<int64_t> time;
  std::optional<std::string> state;
  std::optional<std::string> unit_name;
  JSONPrimitive characteristic;

  [[nodiscard]] json to_json() const;
  static DeviceCharacteristic from_json(const json &json);
};

} // namespace falcon::database
