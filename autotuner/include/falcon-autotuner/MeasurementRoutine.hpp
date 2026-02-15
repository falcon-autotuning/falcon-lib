#pragma once

#include "ParameterMap.hpp"
#include <memory>
#include <string>

namespace falcon {
namespace autotuner {

/**
 * @brief Result of a measurement execution
 */
struct MeasurementResult {
  bool success = false;
  std::string error_message;
  ParameterMap outputs;
};

/**
 * @brief Base class for measurement routines
 */
class MeasurementRoutine {
public:
  virtual ~MeasurementRoutine() = default;

  /**
   * @brief Execute the measurement
   * @param inputs Input parameters
   * @return Result containing success status and output parameters
   */
  virtual MeasurementResult execute(const ParameterMap &inputs) = 0;

  /**
   * @brief Get the name of this measurement
   */
  virtual std::string name() const = 0;
};

/**
 * @brief Functional measurement (wraps a lambda/function)
 */
class FunctionalMeasurement : public MeasurementRoutine {
public:
  using Func = std::function<MeasurementResult(const ParameterMap &)>;

  FunctionalMeasurement(std::string name, Func func)
      : name_(std::move(name)), func_(std::move(func)) {}

  MeasurementResult execute(const ParameterMap &inputs) override {
    return func_(inputs);
  }

  std::string name() const override { return name_; }

private:
  std::string name_;
  Func func_;
};

} // namespace autotuner
} // namespace falcon
