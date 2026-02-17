#include "DatabaseTestFixture.hpp"
#include "falcon-database/SnapshotManager.hpp"
#include <filesystem>
#include <gtest/gtest.h>

using namespace falcon::database;
using namespace falcon::database::test;

class FullWorkflowTest : public DatabaseTestFixture {
protected:
  void SetUp() override { DatabaseTestFixture::SetUp(); }
};

TEST_F(FullWorkflowTest, CompleteDeviceCharacterizationWorkflow) {
  // Step 1: Initialize and insert characteristics
  std::vector<DeviceCharacteristic> characteristics;

  for (int i = 0; i < 10; ++i) {
    DeviceCharacteristic dchar;
    dchar.scope = "quantum_dot_" + std::to_string(i / 3);
    dchar.name = "gate_voltage_" + std::to_string(i);
    dchar.barrier_gate = "BG" + std::to_string(i % 3);
    dchar.plunger_gate = "PG" + std::to_string(i);
    dchar.hash = "21." + std::to_string(i);
    dchar.time = 1700000000 + (i * 100);
    dchar.state = (i % 2 == 0) ? "tuned" : "untuned";
    dchar.unit_name = "mV";
    dchar.uncertainty = 0.05 * (i + 1);
    dchar.characteristic = (100.0 + i * 5.5);

    db_->insert(dchar);
    characteristics.push_back(dchar);
  }

  EXPECT_EQ(db_->count(), 10);

  // Step 2: Query specific characteristics
  auto result = db_->get_by_name("gate_voltage_5");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->plunger_gate, "PG5");
  EXPECT_DOUBLE_EQ(result->characteristic, 127.5);

  // Step 3: Query multiple characteristics
  std::vector<std::string> names = {"gate_voltage_0", "gate_voltage_5",
                                    "gate_voltage_9"};
  auto many_results = db_->get_many(names);
  EXPECT_EQ(many_results.size(), 3);

  // Step 4: Query by hash range
  auto hash_results = db_->get_by_hash_range("21.3", "21.7");
  EXPECT_GE(hash_results.size(), 1);

  // Step 5: Export snapshot
  auto snapshot_path =
      std::filesystem::temp_directory_path() / "workflow_snapshot.json";
  SnapshotManager mgr(db_);
  mgr.export_to_json(snapshot_path.string());
  EXPECT_TRUE(std::filesystem::exists(snapshot_path));

  // Step 6: Validate snapshot
  EXPECT_TRUE(SnapshotManager::validate_snapshot(snapshot_path.string()));

  // Step 7: Delete some characteristics
  db_->delete_by_name("gate_voltage_3");
  EXPECT_EQ(db_->count(), 9);

  // Step 8: Restore from snapshot
  mgr.import_from_json(snapshot_path.string(), true);
  EXPECT_EQ(db_->count(), 10);

  // Step 9: Verify restoration
  auto restored = db_->get_by_name("gate_voltage_3");
  ASSERT_TRUE(restored.has_value());

  // Cleanup
  if (std::filesystem::exists(snapshot_path)) {
    std::filesystem::remove(snapshot_path);
  }
}

TEST_F(FullWorkflowTest, DifferentCharacteristicTypes) {
  // Boolean
  DeviceCharacteristic dchar_bool;
  dchar_bool.name = "is_tuned";
  dchar_bool.hash = "3.2";
  dchar_bool.time = 1000;
  dchar_bool.characteristic = (true);
  db_->insert(dchar_bool);

  // Integer
  DeviceCharacteristic dchar_int;
  dchar_int.name = "step_count";
  dchar_int.hash = "3.2";
  dchar_int.time = 2000;
  dchar_int.characteristic = (int64_t(42));
  db_->insert(dchar_int);

  // Double
  DeviceCharacteristic dchar_double;
  dchar_double.name = "voltage";
  dchar_double.hash = "3.2";
  dchar_double.time = 3000;
  dchar_double.characteristic = (3.14159);
  db_->insert(dchar_double);

  // String
  DeviceCharacteristic dchar_string;
  dchar_string.name = "config_id";
  dchar_string.hash = "3.2";
  dchar_string.time = 4000;
  dchar_string.characteristic = ("CONFIG_A1");
  db_->insert(dchar_string);

  // Retrieve and validate
  auto r_bool = db_->get_by_name("is_tuned");
  ASSERT_TRUE(r_bool.has_value());
  EXPECT_TRUE(r_bool->characteristic);

  auto r_int = db_->get_by_name("step_count");
  ASSERT_TRUE(r_int.has_value());
  EXPECT_EQ(r_int->characteristic, 42);

  auto r_double = db_->get_by_name("voltage");
  ASSERT_TRUE(r_double.has_value());
  EXPECT_DOUBLE_EQ(r_double->characteristic, 3.14159);

  auto r_string = db_->get_by_name("config_id");
  ASSERT_TRUE(r_string.has_value());
  EXPECT_EQ(r_string->characteristic, "CONFIG_A1");
}
