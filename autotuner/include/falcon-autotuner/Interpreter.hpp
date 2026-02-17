#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include "falcon_core/physics/config/core/Config.hpp"
#include <falcon-comms/runtime_comms.hpp>
#include <falcon-database/DatabaseConnection.hpp>
#include <map>
#include <nats/nats.h>
#include <string>

namespace falcon::autotuner {

/**
 * @brief Interprets and executes Falcon DSL autotuners.
 */
class Interpreter {
public:
  explicit Interpreter(const atc::Program &prog);
  ~Interpreter();

  /**
   * @brief Run the autotuner with the given name.
   */
  bool run(const std::string &autotuner_name, ParameterMap &params);

private:
  comms::RuntimeComms comms_;
  database::ReadWriteDatabaseConnection db_;
  const atc::Program &program_;
  falcon_core::physics::config::core::ConfigSP config_;

  struct Context {
    ParameterMap local_params;
    const atc::AutotunerDecl *current_at = nullptr;
    const atc::StateDecl *current_state = nullptr;
    // For loop state tracking: variable name -> current index/value
    std::map<std::string, size_t> loop_indices;
  };

  bool execute_state(Context &ctx);
  const atc::AutotunerDecl *find_autotuner(const std::string &name);
  const atc::StateDecl *find_state(const atc::AutotunerDecl &at,
                                   const std::string &state_name);
};

} // namespace falcon::autotuner
