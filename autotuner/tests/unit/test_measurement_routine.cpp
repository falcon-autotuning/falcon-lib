#include "TestMeasurements.hpp"
#include "falcon-autotuner/MeasurementRoutine.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

TEST(MeasurementRoutineTest, FunctionalMeasurement) {
  auto measurement = std::make_shared<FunctionalMeasurement>(
      "test_func", [](const ParameterMap &inputs) {
        MeasurementResult result;
        int a = inputs.get<int>("a");
        int b = inputs.get<int>("b");
        result.outputs.set("sum", a + b);
        return result;
      });

  ParameterMap inputs;
  inputs.set("a", 10);
  inputs.set("b", 20);

  auto result = measurement->execute(inputs);

  EXPECT_TRUE(result.success);
  EXPECT_EQ(result.outputs.get<int>("sum"), 30);
}

TEST(MeasurementRoutineTest, CountingMeasurement) {
  auto measurement = std::make_shared<CountingMeasurement>();

  EXPECT_EQ(measurement->get_count(), 0);

  ParameterMap inputs;
  measurement->execute(inputs);
  EXPECT_EQ(measurement->get_count(), 1);

  measurement->execute(inputs);
  EXPECT_EQ(measurement->get_count(), 2);
}

TEST(MeasurementRoutineTest, ValidationMeasurement) {
  auto measurement = std::make_shared<ValidationMeasurement>(
      std::vector<std::string>{"param1", "param2"});

  ParameterMap inputs;
  inputs.set("param1", 42);

  auto result = measurement->execute(inputs);
  EXPECT_FALSE(result.success);
  EXPECT_EQ(result.next_action, MeasurementResult::Action::ExitFailure);

  inputs.set("param2", 100);
  result = measurement->execute(inputs);
  EXPECT_TRUE(result.success);
}
