#include "falcon-database/SnapshotManager.hpp"
#include "SnapshotSchemaData.hpp" // Generated header
#include <fstream>
#include <iostream>

namespace falcon::database {

SnapshotManager::SnapshotManager(std::shared_ptr<DatabaseConnection> dconn)
    : db_(std::move(dconn)) {}

void SnapshotManager::export_to_json(const std::string &filename) {
  try {
    // Get all characteristics from the database
    auto characteristics = db_->get_all();
    json output = json::array();
    for (const auto &dchar : characteristics) {
      output.push_back(dchar.to_json());
    }
    std::ofstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file for writing: " + filename);
    }

    file << output.dump(2); // Pretty print with 2-space indent
    file.close();

    std::cout << "Exported " << characteristics.size()
              << " characteristics to: " << filename << '\n';

  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Export error: ") + e.what());
  }
}

void SnapshotManager::import_from_json(const std::string &filename,
                                       bool clear_existing) {
  try {
    std::ifstream file(filename);
    if (!file.is_open()) {
      throw std::runtime_error("Failed to open file for reading: " + filename);
    }
    json json;
    file >> json;
    file.close();
    if (!json.is_array()) {
      throw std::runtime_error("Invalid snapshot format: expected array");
    }

    if (clear_existing) {
      db_->clear_all();
    }

    size_t count = 0;
    for (const auto &item : json) {
      auto dchar = DeviceCharacteristic::from_json(item);
      db_->insert(dchar);
      ++count;
    }

    std::cout << "Imported " << count << " characteristics from: " << filename
              << '\n';

  } catch (const std::exception &e) {
    throw std::runtime_error(std::string("Import error: ") + e.what());
  }
}

std::string SnapshotManager::get_json_schema() {
  // Return the embedded schema
  return std::string(embedded::snapshot_schema);
}

bool SnapshotManager::validate_snapshot(const std::string &filename) {
  try {
    std::ifstream file(filename);
    if (!file.is_open()) {
      return false;
    }
    json json;
    file >> json;
    file.close();
    if (!json.is_array()) {
      return false;
    }

    return std::all_of(json.begin(), json.end(), [](const auto &item) {
      return item.is_object() && item.contains("name") &&
             item.contains("hash") && item.contains("recordtime");
    });

  } catch (...) {
    return false;
  }
}

} // namespace falcon::database
