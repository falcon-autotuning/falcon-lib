#pragma once

#include "falcon-database/DeviceCharacteristic.hpp"
#include <pqxx/pqxx>
namespace falcon::database {

/**
 * @brief Database connection with read-only operations. Useful for contexts
 * where writes are not needed or should be prevented.
 */
class ReadOnlyDatabaseConnection {
public:
  explicit ReadOnlyDatabaseConnection(const std::string &connection_string);
  virtual ~ReadOnlyDatabaseConnection();

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

protected:
  std::unique_ptr<pqxx::connection> conn_;
};

/**
 * @brief Database connection with read/write operations.
 */
class ReadWriteDatabaseConnection : public ReadOnlyDatabaseConnection {
public:
  explicit ReadWriteDatabaseConnection(const std::string &connection_string);
  ~ReadWriteDatabaseConnection() override;

  /**
   * @brief Insert a device characteristic into the database
   */
  void insert(const DeviceCharacteristic &dchar);

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
};
/**
 * @brief Database connection with administrative operations. This can include
 * schema creation, migrations, and other maintenance tasks. It inherits from
 * ReadWriteDatabaseConnection to allow full access.
 */
class AdminDatabaseConnection : public ReadWriteDatabaseConnection {
public:
  explicit AdminDatabaseConnection(const std::string &connection_string);
  ~AdminDatabaseConnection() override;

  /**
   * @brief Initialize the database schema
   */
  void initialize_schema();

  /**
   * @brief Clear all characteristics from the database
   */
  void clear_all();
};
} // namespace falcon::database
