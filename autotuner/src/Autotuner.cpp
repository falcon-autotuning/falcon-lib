#include "falcon-autotuner/Autotuner.hpp"

namespace falcon {
namespace autotuner {

AutotunerResult Autotuner::run(ParameterMap initial_params,
                               int max_transitions) {
  AutotunerResult result;

  if (entry_state_.autotuner_name.empty()) {
    result.success = false;
    result.error_message = "No entry state set for autotuner " + name_;
    return result;
  }

  auto &registry = AutotunerRegistry::instance();

  // Start with initial parameters
  ParameterMap current_params = std::move(initial_params);
  StateId current_state = entry_state_;
  int transitions = 0;

  while (transitions < max_transitions) {
    // Get current state
    auto state = registry.get_state(current_state);
    if (!state) {
      result.success = false;
      result.error_message = "State not found: " + current_state.full_name();
      result.final_state = current_state;
      result.transition_count = transitions;
      return result;
    }

    // Execute measurement if present
    if (state->measurement()) {
      auto measurement_result = state->measurement()->execute(current_params);

      if (!measurement_result.success) {
        result.success = false;
        result.error_message =
            "Measurement failed: " + measurement_result.error_message;
        result.final_state = current_state;
        result.final_params = current_params;
        result.transition_count = transitions;
        return result;
      }

      // Merge measurement outputs into current params
      current_params.merge(measurement_result.outputs);
    }

    // Check if terminal
    if (state->is_terminal()) {
      result.success = true;
      result.final_state = current_state;
      result.final_params = current_params;
      result.transition_count = transitions;
      return result;
    }

    // Evaluate transitions
    auto next_state = state->evaluate_transitions(current_params);
    if (!next_state) {
      result.success = false;
      result.error_message =
          "No valid transition from state " + current_state.full_name();
      result.final_state = current_state;
      result.final_params = current_params;
      result.transition_count = transitions;
      return result;
    }

    // Transition
    current_state = *next_state;
    transitions++;
  }

  // Hit max transitions
  result.success = false;
  result.error_message =
      "Exceeded maximum transitions (" + std::to_string(max_transitions) + ")";
  result.final_state = current_state;
  result.final_params = current_params;
  result.transition_count = transitions;
  return result;
}

} // namespace autotuner
} // namespace falcon
