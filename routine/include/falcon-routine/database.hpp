#pragma once
#include "falcon-database/DatabaseConnection.hpp"
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace falcon::routine {

/**
 * @brief Read-only database connection with lazy connection establishment.
 * Connects to the database on first use of any method.
 * Thread-safe.
 */
class LazyReadOnlyDatabaseConnection
    : public falcon::database::ReadOnlyDatabaseConnection {
public:
  explicit LazyReadOnlyDatabaseConnection(std::string connection_string)
      : falcon::database::ReadOnlyDatabaseConnection(""),
        conn_str_(std::move(connection_string)) {}

  std::optional<falcon::database::DeviceCharacteristic>
  get_by_name(const std::string &name) {
    ensure_connected();
    return falcon::database::ReadOnlyDatabaseConnection::get_by_name(name);
  }
  std::vector<falcon::database::DeviceCharacteristic>
  get_many(const std::vector<std::string> &names) {
    ensure_connected();
    return falcon::database::ReadOnlyDatabaseConnection::get_many(names);
  }
  std::vector<falcon::database::DeviceCharacteristic>
  get_by_hash_range(const std::string &hash_start,
                    const std::string &hash_end) {
    ensure_connected();
    return falcon::database::ReadOnlyDatabaseConnection::get_by_hash_range(
        hash_start, hash_end);
  }
  size_t count() {
    ensure_connected();
    return falcon::database::ReadOnlyDatabaseConnection::count();
  }
  bool test_connection() {
    ensure_connected();
    return falcon::database::ReadOnlyDatabaseConnection::test_connection();
  }
  std::vector<falcon::database::DeviceCharacteristic> get_all() {
    ensure_connected();
    return falcon::database::ReadOnlyDatabaseConnection::get_all();
  }
  std::vector<falcon::database::DeviceCharacteristic>
  get_by_query(const falcon::database::DeviceCharacteristicQuery &query) {
    ensure_connected();
    return falcon::database::ReadOnlyDatabaseConnection::get_by_query(query);
  }
  [[nodiscard]] bool is_connected() const { return connected_; }

private:
  void ensure_connected() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_) {
      conn_ = std::make_unique<pqxx::connection>(conn_str_);
      connected_ = true;
    }
  }
  std::string conn_str_;
  bool connected_{};
  std::mutex mutex_;
};

} // namespace falcon::routine
