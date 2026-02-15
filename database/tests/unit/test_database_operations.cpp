#include "DatabaseTestFixture.hpp"
#include <chrono>
#include <gtest/gtest.h>
using namespace falcon::database;
using namespace falcon::database::test;

class DatabaseOperationsTest : public DatabaseTestFixture {};

TEST_F(DatabaseOperationsTest, InsertAndRetrieve) {
  DeviceCharacteristic dchar;
  dchar.scope = "gate_voltages";
  dchar.name = "plunger_voltage";
  dchar.plunger_gate = std::make_optional<std::string>("P1");
  dchar.hash = std::make_optional<std::string>("98.32");
  dchar.time = std::make_optional<int64_t>(
      std::chrono::system_clock::now().time_since_epoch().count());
  dchar.state = std::make_optional<std::string>("tuned");
  dchar.unit_name = std::make_optional<std::string>("mV");
  dchar.characteristic = JSONPrimitive(150.5);
  dchar.uncertainty = std::make_optional<double>(0.02);

  db_->insert(dchar);

  auto retrieved = db_->get_by_name("plunger_voltage");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->name, "plunger_voltage");
  ASSERT_TRUE(retrieved->hash.has_value());
  EXPECT_EQ(*retrieved->hash, "98.32");
  EXPECT_DOUBLE_EQ(retrieved->characteristic.as_double(), 150.5);
  ASSERT_TRUE(retrieved->plunger_gate.has_value());
  EXPECT_EQ(*retrieved->plunger_gate, "P1");
  ASSERT_TRUE(retrieved->unit_name.has_value());
  EXPECT_EQ(*retrieved->unit_name, "mV");
  ASSERT_TRUE(retrieved->uncertainty.has_value());
  EXPECT_DOUBLE_EQ(*retrieved->uncertainty, 0.02);
}

TEST_F(DatabaseOperationsTest, InsertWithOptionalsUnset) {
  DeviceCharacteristic dchar;
  dchar.scope = "test_scope";
  dchar.name = "optional_test";
  dchar.characteristic = JSONPrimitive(42.0);
  // All other optionals left unset
  db_->insert(dchar);

  auto retrieved = db_->get_by_name("optional_test");
  ASSERT_TRUE(retrieved.has_value());
  EXPECT_EQ(retrieved->name, "optional_test");
  EXPECT_FALSE(retrieved->hash.has_value());
  EXPECT_FALSE(retrieved->plunger_gate.has_value());
  EXPECT_FALSE(retrieved->uncertainty.has_value());
}

TEST_F(DatabaseOperationsTest, GetByQuery) {
  DeviceCharacteristic dchar;
  dchar.scope = "query_scope";
  dchar.name = "query_name";
  dchar.hash = std::make_optional<std::string>("9.8.7");
  dchar.characteristic = JSONPrimitive(123.0);
  db_->insert(dchar);

  DeviceCharacteristicQuery query;
  query.scope = "query_scope";
  query.name = "query_name";
  query.hash = "9.8.7";
  auto results = db_->get_by_query(query);
  ASSERT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].name, "query_name");
  ASSERT_TRUE(results[0].hash.has_value());
  EXPECT_EQ(*results[0].hash, "9.8.7");
}

TEST_F(DatabaseOperationsTest, GetNonExistent) {
  auto result = db_->get_by_name("does_not_exist");
  EXPECT_FALSE(result.has_value());
}

TEST_F(DatabaseOperationsTest, DeleteByName) {
  DeviceCharacteristic dchar;
  dchar.name = "to_delete";
  dchar.hash = std::make_optional<std::string>("hash_del");
  dchar.time = std::make_optional<int64_t>(1000);
  dchar.characteristic = JSONPrimitive(1.0);
  db_->insert(dchar);
  EXPECT_TRUE(db_->get_by_name("to_delete").has_value());
  bool deleted = db_->delete_by_name("to_delete");
  EXPECT_TRUE(deleted);
  EXPECT_FALSE(db_->get_by_name("to_delete").has_value());
}

TEST_F(DatabaseOperationsTest, DeleteByHash) {
  DeviceCharacteristic dc1;
  dc1.name = "char1";
  dc1.hash = std::make_optional<std::string>("same_hash");
  dc1.time = std::make_optional<int64_t>(1000);
  dc1.characteristic = JSONPrimitive(1.0);
  DeviceCharacteristic dc2;
  dc2.name = "char2";
  dc2.hash = std::make_optional<std::string>("same_hash");
  dc2.time = std::make_optional<int64_t>(2000);
  dc2.characteristic = JSONPrimitive(2.0);
  db_->insert(dc1);
  db_->insert(dc2);
  int deleted = db_->delete_by_hash("same_hash");
  EXPECT_EQ(deleted, 2);
}

TEST_F(DatabaseOperationsTest, GetMany) {
  for (int i = 0; i < 5; ++i) {
    DeviceCharacteristic dchar;
    dchar.name = "char_" + std::to_string(i);
    dchar.hash = std::make_optional<std::string>("hash_" + std::to_string(i));
    dchar.time = std::make_optional<int64_t>(1000 + i);
    dchar.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dchar);
  }
  std::vector<std::string> names = {"char_0", "char_2", "char_4"};
  auto results = db_->get_many(names);
  EXPECT_EQ(results.size(), 3);
}

TEST_F(DatabaseOperationsTest, GetByHashRange) {
  for (int i = 0; i < 10; ++i) {
    DeviceCharacteristic dchar;
    dchar.name = "char_" + std::to_string(i);
    dchar.hash = std::make_optional<std::string>(std::to_string(i) + ".0.0");
    dchar.time = std::make_optional<int64_t>(1000 + i);
    dchar.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dchar);
  }
  auto results = db_->get_by_hash_range("2.0.0", "7.0.0");
  EXPECT_GE(results.size(), 1);
  for (const auto &dchar : results) {
    ASSERT_TRUE(dchar.hash.has_value());
    // Parse hash as vector of ints for comparison
    auto parse_hash = [](const std::string &h) {
      std::vector<int> v;
      std::stringstream ss(h);
      std::string item;
      while (std::getline(ss, item, '.'))
        v.push_back(std::stoi(item));
      return v;
    };
    auto h = parse_hash(*dchar.hash);
    auto h_start = parse_hash("2.0.0");
    auto h_end = parse_hash("7.0.0");
    EXPECT_GE(h, h_start);
    EXPECT_LE(h, h_end);
  }
}

TEST_F(DatabaseOperationsTest, GetByQueryWildcard) {
  DeviceCharacteristic dchar;
  dchar.scope = "wildcard_scope";
  dchar.name = "wildcard_name";
  dchar.characteristic = JSONPrimitive(77.0);
  db_->insert(dchar);

  DeviceCharacteristicQuery query;
  query.scope = "wildcard_scope";
  // name left unset (wildcard)
  auto results = db_->get_by_query(query);
  ASSERT_GE(results.size(), 1);
  bool found = false;
  for (const auto &d : results) {
    if (d.name == "wildcard_name")
      found = true;
  }
  EXPECT_TRUE(found);
}

TEST_F(DatabaseOperationsTest, ClearAll) {
  for (int i = 0; i < 10; ++i) {
    DeviceCharacteristic dchar;
    dchar.name = "char_" + std::to_string(i);
    dchar.hash = std::make_optional<std::string>("hash_" + std::to_string(i));
    dchar.time = std::make_optional<int64_t>(1000 + i);
    dchar.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dchar);
  }
  EXPECT_EQ(db_->count(), 10);
  db_->clear_all();
  EXPECT_EQ(db_->count(), 0);
}

TEST_F(DatabaseOperationsTest, Count) {
  EXPECT_EQ(db_->count(), 0);
  for (int i = 0; i < 7; ++i) {
    DeviceCharacteristic dchar;
    dchar.name = "char_" + std::to_string(i);
    dchar.hash = std::make_optional<std::string>("hash_" + std::to_string(i));
    dchar.time = std::make_optional<int64_t>(1000 + i);
    dchar.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dchar);
  }
  EXPECT_EQ(db_->count(), 7);
}
