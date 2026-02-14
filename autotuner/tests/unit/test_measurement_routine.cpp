#include "TestMeasurements.hpp"
#include "falcon-autotuner/MeasurementRoutine.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;
/**
 * @brief Functional measurement routine using lambdas
 */
class FunctionalMeasurement : public MeasurementRoutine {
public:
  using MeasurementFunction =
      std::function<MeasurementResult(const ParameterMap &)>;

  FunctionalMeasurement(std::string name, MeasurementFunction func)
      : name_(std::move(name)), func_(std::move(func)) {}

  MeasurementResult execute(const ParameterMap &inputs) override {
    return func_(inputs);
  }

  [[nodiscard]] std::string name() const override { return name_; }

private:
  std::string name_;
  MeasurementFunction func_;
};
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
