#pragma once

#include "falcon-database/DatabaseConnection.hpp"
#include <string>

namespace falcon::database {

/**
 * @brief Manages JSON snapshots of the database for human-readable
 * backup/restore
 */
class SnapshotManager {
public:
  explicit SnapshotManager(std::shared_ptr<DatabaseConnection> dconn);

  /**
   * @brief Export all device characteristics to a JSON file
   * @param filename Path to output JSON file
   */
  void export_to_json(const std::string &filename);

  /**
   * @brief Import device characteristics from a JSON file
   * @param filename Path to input JSON file
   * @param clear_existing If true, clears existing data before import
   */
  void import_from_json(const std::string &filename,
                        bool clear_existing = false);

  /**
   * @brief Get the JSON schema for the snapshot format
   * @return JSON schema as a string
   */
  static std::string get_json_schema();

  /**
   * @brief Validate a JSON file against the snapshot schema
   * @param filename Path to JSON file to validate
   * @return true if valid, false otherwise
   */
  static bool validate_snapshot(const std::string &filename);

private:
  std::shared_ptr<DatabaseConnection> db_;
};

} // namespace falcon::database
