#pragma once

#include <cstdint>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <utility>
#include <vector>

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
  static JSONPrimitive from_json(const json &j);

private:
  Type type_;
  bool bool_value_ = false;
  int64_t int_value_ = 0;
  double double_value_ = 0.0;
  std::string string_value_;
};

/**
 * @brief Represents a device characteristic with extended metadata
 */
struct DeviceCharacteristic {
  std::string scope;
  std::string name;
  std::string barrier_gate;
  std::string plunger_gate;
  std::string reservoir_gate;
  std::string screening_gate;
  std::string extra;
  double uncertainty = 0.0;
  std::string hash;
  int64_t time = 0; // Unix timestamp
  std::string state;
  std::string unit_name;
  JSONPrimitive characteristic;

  [[nodiscard]] json to_json() const;
  static DeviceCharacteristic from_json(const json &j);
};

/**
 * @brief Database connection and operations for DeviceCharacteristic
 */
class DatabaseConnection {
public:
  explicit DatabaseConnection(const std::string &connection_string);
  ~DatabaseConnection();

  // Delete copy constructor and assignment
  DatabaseConnection(const DatabaseConnection &) = delete;
  DatabaseConnection &operator=(const DatabaseConnection &) = delete;

  // Allow move
  DatabaseConnection(DatabaseConnection &&) noexcept = default;
  DatabaseConnection &operator=(DatabaseConnection &&) noexcept = default;

  /**
   * @brief Initialize the database schema
   */
  void initialize_schema();

  /**
   * @brief Insert a device characteristic into the database
   */
  void insert(const DeviceCharacteristic &dc);

  /**
   * @brief Get a characteristic by name
   * @return Optional DeviceCharacteristic if found
   */
  std::optional<DeviceCharacteristic> get_by_name(const std::string &name);

  /**
   * @brief Get all characteristics matching a set of names
   */
  std::vector<DeviceCharacteristic>
  get_many(const std::vector<std::string> &names);

  /**
   * @brief Get characteristics within a range of hashes
   */
  std::vector<DeviceCharacteristic>
  get_by_hash_range(const std::string &hash_start, const std::string &hash_end);

  /**
   * @brief Delete a characteristic by name
   * @return true if deleted, false if not found
   */
  bool delete_by_name(const std::string &name);

  /**
   * @brief Delete a characteristic by hash
   * @return Number of rows deleted
   */
  int delete_by_hash(const std::string &hash);

  /**
   * @brief Clear all characteristics from the database
   */
  void clear_all();

  /**
   * @brief Get count of all characteristics
   */
  size_t count();

  /**
   * @brief Test database connection
   */
  bool test_connection();

  /**
   * @brief Get all device characteristics from the database
   * @return Vector of all characteristics
   */
  std::vector<DeviceCharacteristic> get_all();

private:
  std::unique_ptr<pqxx::connection> conn_;
};

} // namespace falcon::database
