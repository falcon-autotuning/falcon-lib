#include "DatabaseTestFixture.hpp"
#include <chrono>
#include <gtest/gtest.h>

using namespace falcon::database;
using namespace falcon::database::test;

class DatabaseOperationsTest : public DatabaseTestFixture {};

TEST_F(DatabaseOperationsTest, InsertAndRetrieve) {
  DeviceCharacteristic dc;
  dc.scope = "gate_voltages";
  dc.name = "plunger_voltage";
  dc.plunger_gate = "P1";
  dc.hash = "test_hash_001";
  dc.time = std::chrono::system_clock::now().time_since_epoch().count();
  dc.state = "tuned";
  dc.unit_name = "mV";
  dc.characteristic = JSONPrimitive(150.5);
  dc.uncertainty = 0.02;

  db_->insert(dc);

  auto retrieved = db_->get_by_name("plunger_voltage");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->name, "plunger_voltage");
  EXPECT_EQ(retrieved->hash, "test_hash_001");
  EXPECT_DOUBLE_EQ(retrieved->characteristic.as_double(), 150.5);
}

TEST_F(DatabaseOperationsTest, GetNonExistent) {
  auto result = db_->get_by_name("does_not_exist");
  EXPECT_FALSE(result.has_value());
}

TEST_F(DatabaseOperationsTest, DeleteByName) {
  DeviceCharacteristic dc;
  dc.name = "to_delete";
  dc.hash = "hash_del";
  dc.time = 1000;
  dc.characteristic = JSONPrimitive(1.0);

  db_->insert(dc);
  EXPECT_TRUE(db_->get_by_name("to_delete").has_value());

  bool deleted = db_->delete_by_name("to_delete");
  EXPECT_TRUE(deleted);
  EXPECT_FALSE(db_->get_by_name("to_delete").has_value());
}

TEST_F(DatabaseOperationsTest, DeleteByHash) {
  DeviceCharacteristic dc1;
  dc1.name = "char1";
  dc1.hash = "same_hash";
  dc1.time = 1000;
  dc1.characteristic = JSONPrimitive(1.0);

  DeviceCharacteristic dc2;
  dc2.name = "char2";
  dc2.hash = "same_hash";
  dc2.time = 2000;
  dc2.characteristic = JSONPrimitive(2.0);

  db_->insert(dc1);
  db_->insert(dc2);

  int deleted = db_->delete_by_hash("same_hash");
  EXPECT_EQ(deleted, 2);
}

TEST_F(DatabaseOperationsTest, GetMany) {
  for (int i = 0; i < 5; ++i) {
    DeviceCharacteristic dc;
    dc.name = "char_" + std::to_string(i);
    dc.hash = "hash_" + std::to_string(i);
    dc.time = 1000 + i;
    dc.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dc);
  }

  std::vector<std::string> names = {"char_0", "char_2", "char_4"};
  auto results = db_->get_many(names);

  EXPECT_EQ(results.size(), 3);
}

TEST_F(DatabaseOperationsTest, GetByHashRange) {
  for (int i = 0; i < 10; ++i) {
    DeviceCharacteristic dc;
    dc.name = "char_" + std::to_string(i);
    dc.hash = "hash_" + std::string(2 - std::to_string(i).length(), '0') +
              std::to_string(i);
    dc.time = 1000 + i;
    dc.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dc);
  }

  auto results = db_->get_by_hash_range("hash_02", "hash_07");
  EXPECT_GE(results.size(), 1);

  for (const auto &dc : results) {
    EXPECT_GE(dc.hash, "hash_02");
    EXPECT_LE(dc.hash, "hash_07");
  }
}

TEST_F(DatabaseOperationsTest, ClearAll) {
  for (int i = 0; i < 10; ++i) {
    DeviceCharacteristic dc;
    dc.name = "char_" + std::to_string(i);
    dc.hash = "hash_" + std::to_string(i);
    dc.time = 1000 + i;
    dc.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dc);
  }

  EXPECT_EQ(db_->count(), 10);
  db_->clear_all();
  EXPECT_EQ(db_->count(), 0);
}

TEST_F(DatabaseOperationsTest, Count) {
  EXPECT_EQ(db_->count(), 0);

  for (int i = 0; i < 7; ++i) {
    DeviceCharacteristic dc;
    dc.name = "char_" + std::to_string(i);
    dc.hash = "hash_" + std::to_string(i);
    dc.time = 1000 + i;
    dc.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dc);
  }

  EXPECT_EQ(db_->count(), 7);
}
