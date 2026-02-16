#pragma once

#include "DeviceCharacteristic.hpp"
#include <memory>
#include <mutex>
#include <optional>
#include <pqxx/pqxx>
#include <string>
#include <vector>

namespace falcon::database {

/**
 * @brief Read-only database connection with lazy initialization.
 *
 * Connection is established on first database operation, not at construction.
 * Thread-safe.
 *
 * Connection string priority:
 * 1. Explicit connection_string parameter (if not empty)
 * 2. FALCON_DATABASE_URL environment variable
 * 3. Throws error if neither is available
 */
class ReadOnlyDatabaseConnection {
public:
  /**
   * @brief Construct with optional connection string
   * @param connection_string PostgreSQL connection string (empty = use env var)
   *
   * If connection_string is empty, will use FALCON_DATABASE_URL environment
   * variable. Connection is not established until first database operation.
   */
  explicit ReadOnlyDatabaseConnection(
      const std::string &connection_string = "");
  virtual ~ReadOnlyDatabaseConnection();

  /**
   * @brief Get device characteristic by name
   */
  std::optional<DeviceCharacteristic> get_by_name(const std::string &name);

  /**
   * @brief Get multiple device characteristics by names
   */
  std::vector<DeviceCharacteristic>
  get_many(const std::vector<std::string> &names);

  /**
   * @brief Get all device characteristics
   */
  std::vector<DeviceCharacteristic> get_all();

  /**
   * @brief Get device characteristics by hash range
   */
  std::vector<DeviceCharacteristic>
  get_by_hash_range(const std::string &hash_start, const std::string &hash_end);

  /**
   * @brief Query device characteristics with filters
   */
  std::vector<DeviceCharacteristic>
  get_by_query(const DeviceCharacteristicQuery &query);

  /**
   * @brief Count all device characteristics
   */
  size_t count();

  /**
   * @brief Test if connection is alive (establishes connection if needed)
   */
  bool test_connection();

  /**
   * @brief Check if connection has been established
   */
  [[nodiscard]] bool is_connected() const { return connected_; }

protected:
  /**
   * @brief Ensure database connection is established
   * Thread-safe. Idempotent.
   */
  void ensure_connected();

  /**
   * @brief Get the connection string being used
   */
  [[nodiscard]] const std::string &get_connection_string() const {
    return conn_str_;
  }

  std::unique_ptr<pqxx::connection> conn_;
  bool connected_{false};
  mutable std::mutex conn_mutex_;
  std::string conn_str_;
};

/**
 * @brief Database connection with write operations
 */
class ReadWriteDatabaseConnection : public ReadOnlyDatabaseConnection {
public:
  explicit ReadWriteDatabaseConnection(
      const std::string &connection_string = "");
  ~ReadWriteDatabaseConnection() override;

  /**
   * @brief Insert a device characteristic
   */
  void insert(const DeviceCharacteristic &dchar);

  /**
   * @brief Delete device characteristic by name
   * @return true if deleted, false if not found
   */
  bool delete_by_name(const std::string &name);

  /**
   * @brief Delete all device characteristics with given hash
   * @return number of records deleted
   */
  int delete_by_hash(const std::string &hash);
};

/**
 * @brief Database connection with administrative operations
 */
class AdminDatabaseConnection : public ReadWriteDatabaseConnection {
public:
  explicit AdminDatabaseConnection(const std::string &connection_string = "");
  ~AdminDatabaseConnection() override;

  /**
   * @brief Initialize the database schema
   * Establishes connection if not already connected.
   */
  void initialize_schema();

  /**
   * @brief Clear all characteristics from the database
   */
  void clear_all();
};

} // namespace falcon::database
