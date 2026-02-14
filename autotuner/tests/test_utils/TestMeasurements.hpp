#pragma once

#include "falcon-autotuner/MeasurementRoutine.hpp"
#include <atomic>

namespace falcon {
namespace autotuner {
namespace test {

/**
 * @brief Simple measurement that counts executions
 */
class CountingMeasurement : public MeasurementRoutine {
public:
  CountingMeasurement() : count_(0) {}

  MeasurementResult execute(const ParameterMap &inputs) override {
    count_++;

    MeasurementResult result;
    result.outputs.set("count", static_cast<int64_t>(count_.load()));
    result.outputs.set("success", true);
    return result;
  }

  std::string name() const override { return "CountingMeasurement"; }

  int get_count() const { return count_.load(); }

private:
  std::atomic<int> count_;
};

/**
 * @brief Measurement that validates input parameters
 */
class ValidationMeasurement : public MeasurementRoutine {
public:
  explicit ValidationMeasurement(std::vector<std::string> required_params)
      : required_params_(std::move(required_params)) {}

  MeasurementResult execute(const ParameterMap &inputs) override {
    MeasurementResult result;

    for (const auto &param : required_params_) {
      if (!inputs.has(param)) {
        result.success = false;
        result.error_message = "Missing required parameter: " + param;
        result.next_action = MeasurementResult::Action::ExitFailure;
        return result;
      }
    }

    result.outputs.set("validation", "passed");
    return result;
  }

  std::string name() const override { return "ValidationMeasurement"; }

  std::vector<std::string> expected_inputs() const override {
    return required_params_;
  }

private:
  std::vector<std::string> required_params_;
};

/**
 * @brief Measurement that sums input parameters
 */
class SummingMeasurement : public MeasurementRoutine {
public:
  MeasurementResult execute(const ParameterMap &inputs) override {
    MeasurementResult result;

    double sum = 0.0;
    for (const auto &key : inputs.keys()) {
      if (auto val = inputs.try_get<int64_t>(key)) {
        sum += static_cast<double>(*val);
      } else if (auto val = inputs.try_get<double>(key)) {
        sum += *val;
      }
    }

    result.outputs.set("sum", sum);
    return result;
  }

  std::string name() const override { return "SummingMeasurement"; }
};

} // namespace test
} // namespace autotuner
} // namespace falcon
