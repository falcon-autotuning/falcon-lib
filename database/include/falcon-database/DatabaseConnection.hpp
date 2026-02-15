#pragma once

#include "falcon-database/DeviceCharacteristic.hpp"
namespace falcon::database {
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
  void insert(const DeviceCharacteristic &dchar);

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

  /**
   * @brief Get by query from the database. Optional fields are wildcards
   * @return Vector of all characteristics
   */
  std::vector<DeviceCharacteristic>
  get_by_query(const DeviceCharacteristicQuery &query);

private:
  std::unique_ptr<pqxx::connection> conn_;
};
} // namespace falcon::database
