#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>

namespace falcon::autotuner {

/**
 * @brief Function signature for external functions.
 *
 * Returns FunctionResult instead of ParameterMap for type-safe tuple returns.
 */
using ExternalFunction = std::function<FunctionResult(ParameterMap &)>;

/**
 * @brief Information about a loaded routine from .so file.
 */
struct RoutineInfo {
  std::string name;
  std::string library_path;
  ExternalFunction function;
  atc::BuiltinSignature signature; // Reference to unified signature
};

/**
 * @brief Central registry for all callable functions.
 *
 * Three categories:
 * 1. Builtin functions (Config::*, log::*, etc.) - compiled into Falcon
 * 2. User routines - loaded from .so files at runtime
 * 3. Other autotuners - compiled .fal autotuners callable as functions
 *
 * UNIFIED DESIGN: Function signatures are defined once in
 * BuiltinFunctionRegistry and referenced here, ensuring parser and runtime
 * always agree.
 */
class FunctionRegistry {
public:
  FunctionRegistry();

  /**
   * @brief Register a builtin function by name.
   *
   * The signature is looked up from the global BuiltinFunctionRegistry.
   * This ensures compile-time and runtime signatures always match.
   *
   * @param name Function name (e.g., "read", "log::info")
   * @param func Implementation function
   */
  void register_builtin(const std::string &name, ExternalFunction func);

  /**
   * @brief Register a routine loaded from .so file.
   */
  void register_routine(const RoutineInfo &routine);
  /**
   * @brief Register an autotuner loaded from parser.
   */
  void register_autotuner(const atc::BuiltinSignature &sig,
                          ExternalFunction func);

  /**
   * @brief Look up function by name (supports both qualified and simple names).
   */
  ExternalFunction *lookup(const std::string &name);

  /**
   * @brief Get signature for a function.
   *
   * Returns the unified BuiltinSignature from the registry.
   */
  [[nodiscard]] const atc::BuiltinSignature *
  get_signature(const std::string &name) const;

  /**
   * @brief Get routine info.
   */
  [[nodiscard]] const RoutineInfo *
  get_routine_info(const std::string &name) const;

  [[nodiscard]] bool has_function(const std::string &name) const;

  /**
   * @brief Create default registry with all builtins.
   */
  static std::shared_ptr<FunctionRegistry> create_default();

private:
  std::map<std::string, ExternalFunction> functions_;
  std::map<std::string, atc::BuiltinSignature>
      signatures_; // References to unified registry
  std::map<std::string, RoutineInfo> routines_;

  // Keep a reference to the builtin registry so signatures remain valid
  atc::BuiltinFunctionRegistry builtin_registry_;
};

void register_all_builtins(FunctionRegistry &registry);

} // namespace falcon::autotuner
