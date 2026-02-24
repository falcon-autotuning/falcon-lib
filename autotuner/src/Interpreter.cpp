#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/log.hpp"
#include <falcon_core/communications/Time.hpp>
#include <iostream>
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

ParameterMap Interpreter::run(const atc::AutotunerDecl &autotuner,
                              ParameterMap &inputs) {
  // Clear previous state
  variables_.clear();

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
    }
    // Otherwise leave uninitialized (will be set before terminal)
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
    if (!state) {
      throw EvaluationError("Unknown state: " + current_state);
    }

    std::cout << "[Interpreter] Entering state: " << current_state << std::endl;

    auto flow = execute_state(*state, state_params);

    if (flow.type == ControlFlow::Type::Terminal) {
      std::cout << "[Interpreter] Reached terminal state" << std::endl;
      break;
    } else if (flow.type == ControlFlow::Type::Transition) {
      current_state = flow.target_state;
      state_params = flow.parameters;
    } else {
      throw EvaluationError("State must end with transition or terminal: " +
                            state->name);
    }
  }

  // Step 5: Extract and return output parameters
  return extract_outputs(autotuner);
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
  // Set up state input parameters if present
  if (!state.input_parameters.empty()) {
    // Check parameter count matches
    if (input_params.size() != state.input_parameters.size()) {
      // Check if we can use default values for missing parameters
      if (input_params.size() < state.input_parameters.size()) {
        // Try to fill in with defaults
        for (size_t i = 0; i < state.input_parameters.size(); ++i) {
          const auto &param_decl = state.input_parameters[i];

          if (i < input_params.size()) {
            // Use provided parameter
            variables_[param_decl->name] = input_params[i];
          } else if (param_decl->default_value.has_value()) {
            // Use default value
            ExprEvaluator eval(variables_, functions_, types_);
            variables_[param_decl->name] =
                eval.evaluate(*param_decl->default_value.value());
          } else {
            throw EvaluationError("State '" + state.name +
                                  "' requires parameter at position " +
                                  std::to_string(i) + ": " + param_decl->name);
          }
        }
      } else {
        throw EvaluationError("State '" + state.name + "' expects " +
                              std::to_string(state.input_parameters.size()) +
                              " parameters but got " +
                              std::to_string(input_params.size()));
      }
    } else {
      // Parameter count matches exactly
      for (size_t i = 0; i < state.input_parameters.size(); ++i) {
        variables_[state.input_parameters[i]->name] = input_params[i];
      }
    }
  } else if (!input_params.empty()) {
    throw EvaluationError(
        "State '" + state.name + "' does not accept parameters but " +
        std::to_string(input_params.size()) + " were provided");
  }

  // Execute state body
  StmtExecutor executor(variables_, functions_, types_);
  auto flow = executor.execute_block(state.body);

  // Clean up state input parameters (out of scope)
  for (const auto &param_decl : state.input_parameters) {
    variables_.erase(param_decl->name);
  }

  // TODO: Clean up state-local variables
  // (Need to track which variables were declared in this state)

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

ParameterMap Interpreter::extract_outputs(const atc::AutotunerDecl &autotuner) {
  ParameterMap outputs;

  for (const auto &output_param : autotuner.output_params) {
    auto it = variables_.find(output_param->name);
    if (it == variables_.end()) {
      throw EvaluationError("Output parameter not set: " + output_param->name);
    }
    outputs[output_param->name] = it->second;
  }

  return outputs;
}

} // namespace falcon::autotuner
