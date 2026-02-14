#include "falcon-database/SnapshotManager.hpp"
#include "SnapshotSchemaData.hpp" // Generated header
#include <fstream>
#include <iostream>

namespace falcon::database {

SnapshotManager::SnapshotManager(DatabaseConnection &db) : db_(db) {}

void SnapshotManager::export_to_json(const std::string &filename) {
  try {
    // Get all characteristics from the database
    auto characteristics = db_.get_all();

    json output = json::array();
    for (const auto &dc : characteristics) {
      output.push_back(dc.to_json());
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

    json j;
    file >> j;
    file.close();

    if (!j.is_array()) {
      throw std::runtime_error("Invalid snapshot format: expected array");
    }

    if (clear_existing) {
      db_.clear_all();
    }

    size_t count = 0;
    for (const auto &item : j) {
      auto dc = DeviceCharacteristic::from_json(item);
      db_.insert(dc);
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

    json j;
    file >> j;
    file.close();

    if (!j.is_array()) {
      return false;
    }

    for (const auto &item : j) {
      if (!item.is_object())
        return false;
      if (!item.contains("name"))
        return false;
      if (!item.contains("hash"))
        return false;
      if (!item.contains("recordtime"))
        return false;
    }

    return true;
  } catch (...) {
    return false;
  }
}

} // namespace falcon::database
