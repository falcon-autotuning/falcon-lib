#include "TestMeasurements.hpp"
#include "falcon-autotuner/Autotuner.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

TEST(NestedAutotunerTest, SimpleNesting) {
  // Inner autotuner
  auto inner = std::make_shared<Autotuner>("inner");
  auto inner_measurement = std::make_shared<CountingMeasurement>();
  inner->set_root(Autotuner::measurement(inner_measurement));

  // Outer autotuner
  auto outer = std::make_shared<Autotuner>("outer");
  outer->set_root(Autotuner::nested(inner));

  auto result = outer->run();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(inner_measurement->get_count(), 1);
}

TEST(NestedAutotunerTest, NestedWithIteration) {
  // Inner autotuner with iteration
  auto inner = std::make_shared<Autotuner>("inner");
  auto inner_measurement = std::make_shared<CountingMeasurement>();
  inner->set_root(Autotuner::iterate_range(
      "inner_var", 0, 5, 1, Autotuner::measurement(inner_measurement)));

  // Outer autotuner calls inner 3 times
  auto outer = std::make_shared<Autotuner>("outer");
  outer->set_root(
      Autotuner::iterate_range("outer_var", 0, 3, 1, Autotuner::nested(inner)));

  auto result = outer->run();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(inner_measurement->get_count(), 15); // 3 * 5
}

TEST(NestedAutotunerTest, ParentParametersReadonly) {
  // Inner autotuner validates it can read parent params
  auto inner = std::make_shared<Autotuner>("inner");
  auto validation = std::make_shared<ValidationMeasurement>(
      std::vector<std::string>{"parent_param"});
  inner->set_root(Autotuner::measurement(validation));

  // Outer provides parent_param
  auto outer = std::make_shared<Autotuner>("outer");
  outer->set_root(Autotuner::iterate_range("parent_param", 0, 3, 1,
                                           Autotuner::nested(inner)));

  auto result = outer->run();

  EXPECT_TRUE(result.success);
}

TEST(NestedAutotunerTest, DeepNesting) {
  // Level 3 (innermost)
  auto level3 = std::make_shared<Autotuner>("level3");
  auto measurement = std::make_shared<CountingMeasurement>();
  level3->set_root(Autotuner::measurement(measurement));

  // Level 2
  auto level2 = std::make_shared<Autotuner>("level2");
  level2->set_root(
      Autotuner::iterate_range("l2_var", 0, 2, 1, Autotuner::nested(level3)));

  // Level 1 (outermost)
  auto level1 = std::make_shared<Autotuner>("level1");
  level1->set_root(
      Autotuner::iterate_range("l1_var", 0, 3, 1, Autotuner::nested(level2)));

  auto result = level1->run();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(measurement->get_count(), 6); // 3 * 2 * 1
}
