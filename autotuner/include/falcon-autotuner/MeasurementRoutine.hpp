#pragma once

#include "falcon-autotuner/ParameterMap.hpp"
#include <string>
#include <vector>

namespace falcon::autotuner {

/**
 * @brief Result of a measurement routine
 */
struct MeasurementResult {
  ParameterMap outputs;
  bool success = true;
  std::string error_message;

  enum class Action : std::uint8_t {
    Continue,    // Continue in current autotuner
    ExitSuccess, // Exit to parent with success
    ExitFailure  // Exit to parent with failure
  };
  Action next_action = Action::Continue;
};

/**
 * @brief Interface for measurement routines
 *
 * Users should implement this interface to define custom measurements.
 * The measurement will receive combined parameters from the current state
 * (both local and parent parameters).
 */
class MeasurementRoutine {
public:
  virtual ~MeasurementRoutine() = default;

  /**
   * @brief Execute the measurement with given parameters
   * @param inputs Combined parameters from state (local + parent)
   * @return Measurement results including outputs and next action
   */
  virtual MeasurementResult execute(const ParameterMap &inputs) = 0;

  /**
   * @brief Get the name of this measurement routine
   */
  [[nodiscard]] virtual std::string name() const = 0;

  /**
   * @brief Get expected input parameter names (for validation)
   */
  [[nodiscard]] virtual std::vector<std::string> expected_inputs() const {
    return {};
  }

  /**
   * @brief Get expected output parameter names (for documentation)
   */
  [[nodiscard]] virtual std::vector<std::string> expected_outputs() const {
    return {};
  }
};

} // namespace falcon::autotuner
