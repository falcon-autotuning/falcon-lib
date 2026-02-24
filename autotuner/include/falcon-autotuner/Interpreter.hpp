#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/StmtExecutor.hpp"
#include "falcon-autotuner/TypeRegistry.hpp"
#include <falcon-comms/runtime_comms.hpp>
#include <falcon-database/DatabaseConnection.hpp>
#include <falcon_core/physics/config/core/Config.hpp>
#include <memory>
#include <string>

namespace falcon::autotuner {

/**
 * @brief Interprets and executes an autotuner program.
 */
class Interpreter {
public:
  /**
   * @brief Construct interpreter with function and type registries.
   *
   * This will:
   * - Initialize NATS communication
   * - Fetch config from instrument server
   * - Set up database read/write functions
   */
  Interpreter(std::shared_ptr<FunctionRegistry> functions,
              std::shared_ptr<TypeRegistry> types);

  /**
   * @brief Run an autotuner with given input parameters.
   */
  ParameterMap run(const atc::AutotunerDecl &autotuner, ParameterMap &inputs);

  /**
   * @brief Get current variable environment (for debugging).
   */
  const ParameterMap &get_variables() const { return variables_; }

  /**
   * @brief Get the loaded config.
   */
  const falcon_core::physics::config::core::ConfigSP &get_config() const {
    return config_;
  }

private:
  void initialize_variables(const atc::AutotunerDecl &autotuner);
  void set_input_parameters(const atc::AutotunerDecl &autotuner,
                            ParameterMap &inputs);
  ControlFlow execute_state(const atc::StateDecl &state,
                            std::vector<RuntimeValue> &input_param);
  const atc::StateDecl *find_state(const atc::AutotunerDecl &autotuner,
                                   const std::string &name);
  ParameterMap extract_outputs(const atc::AutotunerDecl &autotuner);

  // Initialize NATS and fetch config
  void initialize_config();

  // Set up database read/write in function registry
  void setup_database_functions();

  std::shared_ptr<FunctionRegistry> functions_;
  std::shared_ptr<TypeRegistry> types_; // Added TypeRegistry
  ParameterMap variables_;
  database::ReadWriteDatabaseConnection db_;
  falcon_core::physics::config::core::ConfigSP config_;
  comms::RuntimeComms comms_;
};

} // namespace falcon::autotuner
