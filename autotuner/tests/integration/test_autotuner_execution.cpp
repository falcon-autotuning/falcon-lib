#include "TestMeasurements.hpp"
#include "falcon-autotuner/Autotuner.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

TEST(AutotunerTest, SimpleMeasurement) {
  auto autotuner = std::make_shared<Autotuner>("simple");
  auto measurement = std::make_shared<CountingMeasurement>();

  autotuner->set_root(Autotuner::measurement(measurement));

  auto result = autotuner->run();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(measurement->get_count(), 1);
}

TEST(AutotunerTest, ListIteration) {
  auto autotuner = std::make_shared<Autotuner>("list_iter");
  auto measurement = std::make_shared<SummingMeasurement>();

  std::vector<int> values = {1, 2, 3, 4, 5};
  auto root = Autotuner::iterate_list("value", values,
                                      Autotuner::measurement(measurement));

  autotuner->set_root(root);

  auto result = autotuner->run();

  EXPECT_TRUE(result.success);
}

TEST(AutotunerTest, IntegerRangeIteration) {
  auto autotuner = std::make_shared<Autotuner>("range_iter");
  auto counting = std::make_shared<CountingMeasurement>();

  auto root = Autotuner::iterate_range("index", 0, 10, 1,
                                       Autotuner::measurement(counting));

  autotuner->set_root(root);
  auto result = autotuner->run();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(counting->get_count(), 10);
}

TEST(AutotunerTest, FloatRangeIteration) {
  auto autotuner = std::make_shared<Autotuner>("float_iter");
  auto counting = std::make_shared<CountingMeasurement>();

  auto root = Autotuner::iterate_float_range("voltage", 0.0, 1.0, 10,
                                             Autotuner::measurement(counting));

  autotuner->set_root(root);
  auto result = autotuner->run();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(counting->get_count(), 11); // 0.0, 0.1, ..., 1.0
}

TEST(AutotunerTest, NestedIterators) {
  auto autotuner = std::make_shared<Autotuner>("nested_iters");
  auto counting = std::make_shared<CountingMeasurement>();

  // Nested: 3 x 4 = 12 executions
  auto inner =
      Autotuner::iterate_range("j", 0, 4, 1, Autotuner::measurement(counting));

  auto outer = Autotuner::iterate_range("i", 0, 3, 1, inner);

  autotuner->set_root(outer);
  auto result = autotuner->run();

  EXPECT_TRUE(result.success);
  EXPECT_EQ(counting->get_count(), 12);
}

TEST(AutotunerTest, ParameterPropagation) {
  auto autotuner = std::make_shared<Autotuner>("param_prop");

  auto validation = std::make_shared<ValidationMeasurement>(
      std::vector<std::string>{"outer", "inner"});

  auto inner = Autotuner::iterate_range("inner", 0, 3, 1,
                                        Autotuner::measurement(validation));

  auto outer = Autotuner::iterate_range("outer", 0, 2, 1, inner);

  autotuner->set_root(outer);
  auto result = autotuner->run();

  EXPECT_TRUE(result.success);
}
