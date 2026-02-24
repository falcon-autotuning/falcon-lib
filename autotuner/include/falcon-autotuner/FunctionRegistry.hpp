#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include <dlfcn.h>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace falcon::autotuner {

/**
 * @brief Function signature for external functions.
 */
using ExternalFunction = std::function<ParameterMap(ParameterMap &)>;

/**
 * @brief Information about a loaded routine from .so file.
 */
struct RoutineInfo {
  std::string name;
  std::string namespace_name; // Autotuner namespace
  std::vector<std::unique_ptr<atc::ParamDecl>> input_params;
  std::vector<std::unique_ptr<atc::ParamDecl>> output_params;
  void *library_handle;
  ExternalFunction function;
};

/**
 * @brief Central registry for all callable functions.
 *
 * Three categories:
 * 1. Builtin functions (Config::*, log::*, etc.) - compiled into Falcon
 * 2. User routines - loaded from .so files at runtime
 * 3. Other autotuners - compiled .fal autotuners callable as functions
 */
class FunctionRegistry {
public:
  FunctionRegistry();
  ~FunctionRegistry();

  // Prevent copying (we manage library handles)
  FunctionRegistry(const FunctionRegistry &) = delete;
  FunctionRegistry &operator=(const FunctionRegistry &) = delete;

  /**
   * @brief Register a builtin function (hardcoded in Falcon).
   */
  void register_builtin(const std::string &qualified_name,
                        ExternalFunction func);

  /**
   * @brief Load a user routine from a .so file.
   *
   * Symbol name convention: <namespace>_<routine_name>
   * Example: ConditionalNest_Adder
   */
  void load_routine(
      const std::string &routine_name, const std::string &namespace_name,
      const std::string &library_path,
      const std::vector<std::unique_ptr<atc::ParamDecl>> &input_params,
      const std::vector<std::unique_ptr<atc::ParamDecl>> &output_params);

  /**
   * @brief Register an autotuner as a callable function.
   */
  void register_autotuner(const std::string &name, ExternalFunction func);

  /**
   * @brief Look up qualified function (Type::function).
   */
  ExternalFunction *lookup_qualified(const std::string &qualified_name);

  /**
   * @brief Look up simple function (routine or autotuner).
   */
  ExternalFunction *lookup_simple(const std::string &name);

  /**
   * @brief Get routine info.
   */
  const RoutineInfo *get_routine_info(const std::string &name) const;

  bool has_qualified(const std::string &qualified_name) const;
  bool has_simple(const std::string &name) const;

  /**
   * @brief Create default registry with all builtins.
   */
  static std::shared_ptr<FunctionRegistry> create_default();

private:
  std::map<std::string, ExternalFunction> builtin_functions_;
  std::map<std::string, RoutineInfo> user_routines_;
  std::map<std::string, ExternalFunction> autotuner_functions_;
  std::vector<void *> loaded_libraries_;
};

void register_all_builtins(FunctionRegistry &registry);

} // namespace falcon::autotuner
