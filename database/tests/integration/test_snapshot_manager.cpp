#include "DatabaseTestFixture.hpp"
#include "falcon-database/SnapshotManager.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace falcon::database;
using namespace falcon::database::test;

class SnapshotManagerTest : public DatabaseTestFixture {
protected:
  void SetUp() override {
    DatabaseTestFixture::SetUp();
    snapshot_mgr_ = std::make_unique<SnapshotManager>(db_);
    test_file_ = std::filesystem::temp_directory_path() / "test_snapshot.json";
  }

  void TearDown() override {
    if (std::filesystem::exists(test_file_)) {
      std::filesystem::remove(test_file_);
    }
    DatabaseTestFixture::TearDown();
  }

  std::unique_ptr<SnapshotManager> snapshot_mgr_;
  std::filesystem::path test_file_;
};

TEST_F(SnapshotManagerTest, GetSchema) {
  auto schema = SnapshotManager::get_json_schema();
  EXPECT_FALSE(schema.empty());
  EXPECT_NE(schema.find("$schema"), std::string::npos);
  EXPECT_NE(schema.find("device_characteristic"), std::string::npos);
}

TEST_F(SnapshotManagerTest, ExportAndImport) {
  // Insert test data
  for (int i = 0; i < 5; ++i) {
    DeviceCharacteristic dchar;
    dchar.name = "test_" + std::to_string(i);
    dchar.hash = "hash_" + std::to_string(i);
    dchar.time = 1000 + i;
    dchar.scope = "test_scope";
    dchar.plunger_gate = "P" + std::to_string(i);
    dchar.characteristic = (static_cast<double>(i) * 10.0);
    dchar.uncertainty = 0.01;
    db_->insert(dchar);
  }

  EXPECT_EQ(db_->count(), 5);

  // Export
  snapshot_mgr_->export_to_json(test_file_.string());
  EXPECT_TRUE(std::filesystem::exists(test_file_));

  // Clear database
  db_->clear_all();
  EXPECT_EQ(db_->count(), 0);

  // Import
  snapshot_mgr_->import_from_json(test_file_.string());
  EXPECT_EQ(db_->count(), 5);

  // Verify data
  auto retrieved = db_->get_by_name("test_3");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->hash, "hash_3");
  EXPECT_DOUBLE_EQ(retrieved->characteristic, 30.0);
}

TEST_F(SnapshotManagerTest, ValidateSnapshot) {
  // Create a valid snapshot
  json snapshot = json::array();
  for (int i = 0; i < 3; ++i) {
    snapshot.push_back(json{{"name", "test_" + std::to_string(i)},
                            {"hash", "hash_" + std::to_string(i)},
                            {"recordtime", 1000 + i},
                            {"scope", "test"},
                            {"uncertainty", 0.0},
                            {"device_characteristic", i * 5.0}});
  }

  std::ofstream file(test_file_);
  file << snapshot.dump(2);
  file.close();

  EXPECT_TRUE(SnapshotManager::validate_snapshot(test_file_.string()));
}

TEST_F(SnapshotManagerTest, ValidateInvalidSnapshot) {
  // Create an invalid snapshot (missing required fields)
  json snapshot = json::array();
  snapshot.push_back(json{
      {"name", "incomplete"} // Missing hash and recordtime
  });

  std::ofstream file(test_file_);
  file << snapshot.dump(2);
  file.close();

  EXPECT_FALSE(SnapshotManager::validate_snapshot(test_file_.string()));
}
