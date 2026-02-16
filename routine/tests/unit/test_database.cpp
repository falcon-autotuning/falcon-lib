#include "../test_utils/TestFixture.hpp"
#include "falcon-routine/database.hpp"
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::routine;
using namespace falcon::routine::test;
using namespace falcon::database;

class DatabaseUnitTest : public DatabaseTestFixture {};

TEST_F(DatabaseUnitTest, LazyConnectionEstablishment) {
  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());

  // Connection should not be established yet
  // (We can't directly check this, but it shouldn't throw)

  // First operation should establish connection
  auto result = lazy_db.get_by_name("nonexistent");
  EXPECT_FALSE(result.has_value());

  // Should be connected now
  EXPECT_TRUE(lazy_db.is_connected());
}

TEST_F(DatabaseUnitTest, GetByName) {
  // Insert test data via fixture's db
  DeviceCharacteristic dc;
  dc.scope = "test";
  dc.name = "test_device";
  dc.characteristic = JSONPrimitive("test_value");

  db_->insert(dc);

  // Read via lazy connection
  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());
  auto result = lazy_db.get_by_name("test_device");

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->name, "test_device");
  EXPECT_EQ(result->scope, "test");
}

TEST_F(DatabaseUnitTest, GetMany) {
  // Insert multiple test records
  for (int i = 0; i < 5; i++) {
    DeviceCharacteristic dc;
    dc.scope = "test";
    dc.name = "device_" + std::to_string(i);
    dc.characteristic = JSONPrimitive(static_cast<double>(i));
    db_->insert(dc);
  }

  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());
  std::vector<std::string> names = {"device_0", "device_2", "device_4"};

  auto results = lazy_db.get_many(names);

  EXPECT_EQ(results.size(), 3);
}

TEST_F(DatabaseUnitTest, GetAll) {
  // Insert test data
  for (int i = 0; i < 3; i++) {
    DeviceCharacteristic dc;
    dc.scope = "test";
    dc.name = "device_" + std::to_string(i);
    dc.characteristic = JSONPrimitive(i);
    db_->insert(dc);
  }

  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());
  auto results = lazy_db.get_all();

  EXPECT_EQ(results.size(), 3);
}

TEST_F(DatabaseUnitTest, QueryByScope) {
  // Insert data with different scopes
  DeviceCharacteristic dc1;
  dc1.scope = "calibration";
  dc1.name = "cal_1";
  dc1.characteristic = JSONPrimitive(1.0);
  db_->insert(dc1);

  DeviceCharacteristic dc2;
  dc2.scope = "measurement";
  dc2.name = "meas_1";
  dc2.characteristic = JSONPrimitive(2.0);
  db_->insert(dc2);

  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());

  DeviceCharacteristicQuery query;
  query.scope = "calibration";

  auto results = lazy_db.get_by_query(query);

  EXPECT_EQ(results.size(), 1);
  EXPECT_EQ(results[0].scope, "calibration");
}

TEST_F(DatabaseUnitTest, Count) {
  // Insert test data
  for (int i = 0; i < 7; i++) {
    DeviceCharacteristic dc;
    dc.scope = "test";
    dc.name = "device_" + std::to_string(i);
    dc.characteristic = JSONPrimitive(i);
    db_->insert(dc);
  }

  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());

  EXPECT_EQ(lazy_db.count(), 7);
}

TEST_F(DatabaseUnitTest, ThreadSafety) {
  // Insert test data
  DeviceCharacteristic dc;
  dc.scope = "test";
  dc.name = "shared_device";
  dc.characteristic = JSONPrimitive("shared_value");
  db_->insert(dc);

  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());

  // Multiple threads accessing the same connection
  std::vector<std::thread> threads;
  std::atomic<int> success_count{0};

  for (int i = 0; i < 10; i++) {
    threads.emplace_back([&]() {
      auto result = lazy_db.get_by_name("shared_device");
      if (result.has_value()) {
        success_count++;
      }
    });
  }

  for (auto &t : threads) {
    t.join();
  }

  EXPECT_EQ(success_count, 10);
}
