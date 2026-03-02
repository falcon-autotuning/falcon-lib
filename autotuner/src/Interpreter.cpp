#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/log.hpp"
#include <falcon_core/communications/Time.hpp>
#include <fmt/format.h>
namespace {
const int INSTRUMENT_SERVER_LATENCY = 1000;
}
namespace falcon::autotuner {
using falcon_core::communications::Time;
Interpreter::Interpreter(std::shared_ptr<FunctionRegistry> functions,
                         std::shared_ptr<TypeRegistry> types)
    : functions_(std::move(functions)), types_(std::move(types)) {

  log::debug("Interpreter booting up");
  log::debug("Interpreter booted up");
  auto resp = comms_.subscribe_config_response(INSTRUMENT_SERVER_LATENCY,
                                               (int)Time().time());
  config_ = falcon_core::physics::config::core::Config::from_json_string<
      falcon_core::physics::config::core::Config>(resp.response);
}

FunctionResult Interpreter::run(const atc::AutotunerDecl &autotuner,
                                ParameterMap &inputs) {
  // Save the current variable environment (for nested calls)
  ParameterMap saved_variables = std::move(variables_);

  // Clear for this autotuner's execution
  variables_.clear();

  try {
    // Step 1: Initialize autotuner-level variables
    initialize_variables(autotuner);

    // Step 2: Set input parameters
    set_input_parameters(autotuner, inputs);

    // Step 3: Initialize output parameters (to defaults or unset)
    for (const auto &output_param : autotuner.output_params) {
      if (output_param->default_value.has_value()) {
        ExprEvaluator eval(variables_, functions_, types_);
        variables_[output_param->name] =
            eval.evaluate(*output_param->default_value.value());
      } else {
        // Ensure the variable exists so it is treated as a global in states.
        // It will be overwritten by any assignments in states.
        variables_[output_param->name] = RuntimeValue();
      }
    }

    // Step 4: Execute state machine starting from entry state
    std::string current_state = autotuner.entry_state;
    std::vector<RuntimeValue> state_params;

    // Evaluate entry parameters if provided
    if (!autotuner.entry_parameters.empty()) {
      ExprEvaluator eval(variables_, functions_, types_);
      for (const auto &param_expr : autotuner.entry_parameters) {
        state_params.push_back(eval.evaluate(*param_expr));
      }
    }

    // State machine execution loop
    while (true) {
      const atc::StateDecl *state = find_state(autotuner, current_state);
      if (state == nullptr) {
        throw EvaluationError("Unknown state: " + current_state);
      }

      log::debug(fmt::format("Entering state: {}", current_state));

      auto flow = execute_state(*state, state_params);

      if (flow.type == ControlFlow::Type::Terminal) {
        log::debug("Reached terminal state");
        break;
      }
      if (flow.type == ControlFlow::Type::Transition) {
        current_state = flow.target_state;
        state_params = flow.parameters;
      } else {
        throw EvaluationError("State must end with transition or terminal: " +
                              state->name);
      }
    }

    // Step 5: Extract and return output parameters
    FunctionResult result = extract_outputs(autotuner);

    // Restore the parent's variable environment
    variables_ = std::move(saved_variables);

    return result;

  } catch (...) {
    // Restore the parent's variable environment even on error
    variables_ = std::move(saved_variables);
    throw;
  }
}

void Interpreter::initialize_variables(const atc::AutotunerDecl &autotuner) {
  StmtExecutor executor(variables_, functions_, types_);

  for (const auto &var_decl : autotuner.autotuner_variables) {
    executor.execute(*var_decl);
  }
}

void Interpreter::set_input_parameters(const atc::AutotunerDecl &autotuner,
                                       ParameterMap &inputs) {
  for (const auto &input_param : autotuner.input_params) {
    auto it = inputs.find(input_param->name);

    if (it != inputs.end()) {
      // Use provided value
      variables_[input_param->name] = it->second;
    } else if (input_param->default_value.has_value()) {
      // Use default value
      ExprEvaluator eval(variables_, functions_, types_);
      variables_[input_param->name] =
          eval.evaluate(*input_param->default_value.value());
    } else {
      throw EvaluationError("Missing required input parameter: " +
                            input_param->name);
    }
  }
}

ControlFlow
Interpreter::execute_state(const atc::StateDecl &state,
                           std::vector<RuntimeValue> &input_params) {
  // Validate input parameters
  size_t param_count = state.input_parameters.size();
  size_t input_count = input_params.size();

  // variables defined before execution
  std::vector<std::string> globals;
  for (const auto &pair : variables_) {
    globals.push_back(pair.first);
  }

  if (param_count == 0 && input_count > 0) {
    throw EvaluationError("State '" + state.name +
                          "' does not accept parameters but " +
                          std::to_string(input_count) + " were provided");
  }

  if (param_count > 0) {
    if (input_count > param_count) {
      throw EvaluationError(
          "State '" + state.name + "' expects " + std::to_string(param_count) +
          " parameters but got " + std::to_string(input_count));
    }

    for (size_t i = 0; i < param_count; ++i) {
      const auto &param_decl = state.input_parameters[i];
      if (i < input_count) {
        variables_[param_decl->name] = input_params[i];
      } else if (param_decl->default_value.has_value()) {
        ExprEvaluator eval(variables_, functions_, types_);
        variables_[param_decl->name] =
            eval.evaluate(*param_decl->default_value.value());
      } else {
        throw EvaluationError("State '" + state.name +
                              "' requires parameter at position " +
                              std::to_string(i) + ": " + param_decl->name);
      }
    }
  }

  // Execute state body
  StmtExecutor executor(variables_, functions_, types_);
  auto flow = executor.execute_block(state.body);

  // Clean up state-local variables
  for (auto it = variables_.begin(); it != variables_.end();) {
    if (std::find(globals.begin(), globals.end(), it->first) == globals.end()) {
      it = variables_.erase(it);
    } else {
      ++it;
    }
  }

  return flow;
}

const atc::StateDecl *
Interpreter::find_state(const atc::AutotunerDecl &autotuner,
                        const std::string &name) {
  for (const auto &state : autotuner.states) {
    if (state.name == name) {
      return &state;
    }
  }
  return nullptr;
}

FunctionResult
Interpreter::extract_outputs(const atc::AutotunerDecl &autotuner) {
  FunctionResult outputs;

  for (const auto &output_param : autotuner.output_params) {
    auto it = variables_.find(output_param->name);
    if (it == variables_.end()) {
      throw EvaluationError("Output parameter not set: " + output_param->name);
    }
    outputs.push_back(it->second);
  }

  return outputs;
}

} // namespace falcon::autotuner
