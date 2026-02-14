#include "falcon-database/DeviceCharacteristic.hpp"
#include <gtest/gtest.h>

using namespace falcon::database;

TEST(DeviceCharacteristicTest, ToJsonAndBack) {
  DeviceCharacteristic dc;
  dc.scope = "test_scope";
  dc.name = "test_characteristic";
  dc.barrier_gate = "BG1";
  dc.plunger_gate = "PG1";
  dc.reservoir_gate = "RG1";
  dc.screening_gate = "SG1";
  dc.extra = "extra_data";
  dc.uncertainty = 0.05;
  dc.hash = "abc123";
  dc.time = 1234567890;
  dc.state = "initialized";
  dc.unit_name = "mV";
  dc.characteristic = JSONPrimitive(42.5);

  auto j = dc.to_json();
  auto dc2 = DeviceCharacteristic::from_json(j);

  EXPECT_EQ(dc.scope, dc2.scope);
  EXPECT_EQ(dc.name, dc2.name);
  EXPECT_EQ(dc.barrier_gate, dc2.barrier_gate);
  EXPECT_EQ(dc.plunger_gate, dc2.plunger_gate);
  EXPECT_EQ(dc.reservoir_gate, dc2.reservoir_gate);
  EXPECT_EQ(dc.screening_gate, dc2.screening_gate);
  EXPECT_EQ(dc.extra, dc2.extra);
  EXPECT_DOUBLE_EQ(dc.uncertainty, dc2.uncertainty);
  EXPECT_EQ(dc.hash, dc2.hash);
  EXPECT_EQ(dc.time, dc2.time);
  EXPECT_EQ(dc.state, dc2.state);
  EXPECT_EQ(dc.unit_name, dc2.unit_name);
  EXPECT_DOUBLE_EQ(dc.characteristic.as_double(),
                   dc2.characteristic.as_double());
}

TEST(DeviceCharacteristicTest, EmptyFields) {
  DeviceCharacteristic dc;
  dc.name = "minimal";
  dc.hash = "hash1";
  dc.time = 1000;
  dc.characteristic = JSONPrimitive();

  auto j = dc.to_json();
  auto dc2 = DeviceCharacteristic::from_json(j);

  EXPECT_TRUE(dc2.scope.empty());
  EXPECT_TRUE(dc2.barrier_gate.empty());
  EXPECT_EQ(dc2.name, "minimal");
  EXPECT_TRUE(dc2.characteristic.is_null());
}
