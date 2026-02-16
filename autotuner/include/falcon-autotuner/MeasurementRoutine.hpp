#pragma once

#include "falcon-autotuner/ParameterMap.hpp"
#include <string>

namespace falcon::autotuner {

/**
 * @brief Result of a measurement execution
 */
struct MeasurementResult {
  bool success = false;
  ParameterMap outputs;
  std::string error_message;
};

/**
 * @brief Abstract base class for measurement routines
 */
class MeasurementRoutine {
public:
  virtual ~MeasurementRoutine() = default;

  /**
   * @brief Execute the measurement with given inputs
   */
  virtual MeasurementResult execute(const ParameterMap &inputs) = 0;

  /**
   * @brief Get the unique name of this routine
   */
  virtual std::string name() const = 0;
};

} // namespace falcon::autotuner
