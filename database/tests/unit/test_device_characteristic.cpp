#include "falcon-database/DeviceCharacteristic.hpp"
#include <gtest/gtest.h>

using namespace falcon::database;

TEST(DeviceCharacteristicTest, ToJsonAndBack) {
  DeviceCharacteristic dchar;
  dchar.scope = "test_scope";
  dchar.name = "test_characteristic";
  dchar.barrier_gate = "BG1";
  dchar.plunger_gate = "PG1";
  dchar.reservoir_gate = "RG1";
  dchar.screening_gate = "SG1";
  dchar.extra = "extra_data";
  dchar.uncertainty = 0.05;
  dchar.hash = "22.54";
  dchar.time = 1234567890;
  dchar.state = "initialized";
  dchar.unit_name = "mV";
  dchar.characteristic = 42.5;

  auto j = dchar.to_json();
  std::cout << j.dump() << '\n';
  auto dchar2 = DeviceCharacteristic::from_json(j);

  EXPECT_EQ(dchar.scope, dchar2.scope);
  EXPECT_EQ(dchar.name, dchar2.name);
  EXPECT_EQ(dchar.barrier_gate, dchar2.barrier_gate);
  EXPECT_EQ(dchar.plunger_gate, dchar2.plunger_gate);
  EXPECT_EQ(dchar.reservoir_gate, dchar2.reservoir_gate);
  EXPECT_EQ(dchar.screening_gate, dchar2.screening_gate);
  EXPECT_EQ(dchar.extra, dchar2.extra);
  ASSERT_EQ(dchar.uncertainty.has_value(), dchar2.uncertainty.has_value());
  if (dchar.uncertainty && dchar2.uncertainty) {
    EXPECT_DOUBLE_EQ(*dchar.uncertainty, *dchar2.uncertainty);
  }
  EXPECT_EQ(dchar.hash, dchar2.hash);
  EXPECT_EQ(dchar.time, dchar2.time);
  EXPECT_EQ(dchar.state, dchar2.state);
  EXPECT_EQ(dchar.unit_name, dchar2.unit_name);
  EXPECT_DOUBLE_EQ(dchar.characteristic.get<double>(),
                   dchar2.characteristic.get<double>());
}
