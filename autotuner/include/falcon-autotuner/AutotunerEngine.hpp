#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/TypeRegistry.hpp"
#include <memory>
#include <string>
#include <vector>

namespace falcon::autotuner {

/**
 * @brief Information about a loaded routine from .so file.
 */
struct RoutineConfig {
  std::string name;
  std::string library_path;
  std::string name_space;
};
/**
 * @brief Main user-facing API for running autotuners.
 *
 * Workflow:
 * 1. Create AutotunerEngine
 * 2. Load .fal file(s) with autotuner definitions
 * 3. Load user routine libraries (.so files)
 * 4. Run autotuners
 */
class AutotunerEngine {
public:
  AutotunerEngine();

  /**
   * @brief Load and parse a .fal file.
   *
   * Can be called multiple times to load multiple files.
   * Handles multiple autotuners per file.
   *
   * @param fal_file_path Path to .fal file
   * @return true if successful
   */
  bool load_fal_file(const std::string &fal_file_path);

  /**
   * @brief Load a user routine from compiled .so file.
   * @return true if successful
   */
  bool load_routine_library(const RoutineConfig &info);

  /**
   * @brief Run an autotuner.
   *
   * @param autotuner_name Name of autotuner to run
   * @param inputs Input parameter values
   * @return Output parameter values
   */
  FunctionResult run_autotuner(const std::string &autotuner_name,
                               ParameterMap &inputs);

  /**
   * @brief Get list of loaded autotuner names.
   */
  [[nodiscard]] std::vector<std::string> get_loaded_autotuners() const;

  /**
   * @brief Get list of loaded routine names.
   */
  [[nodiscard]] std::vector<std::string> get_loaded_routines() const;

  /**
   * @brief Check if autotuner is loaded.
   */
  [[nodiscard]] bool has_autotuner(const std::string &name) const;

  /**
   * @brief Get autotuner declaration (for inspection).
   */
  [[nodiscard]] const atc::AutotunerDecl *
  get_autotuner(const std::string &name) const;
  /**
   * @brief Load pre-compiled .fal file (faster than parsing).
   */
  bool load_fal_compiled(const std::string &compiled_path);

  /**
   * @brief Save loaded autotuners as pre-compiled binary.
   */
  bool save_fal_compiled(const std::string &output_path);

  /**
   * @brief Get list of declared routines (may not all be loaded yet).
   */
  [[nodiscard]] std::vector<std::string> get_declared_routines() const;

  // Advanced access
  std::shared_ptr<FunctionRegistry> get_function_registry() {
    return function_registry_;
  }
  std::shared_ptr<TypeRegistry> get_type_registry() { return type_registry_; }

private:
  void register_autotuner_as_function(const atc::AutotunerDecl &autotuner);

  std::shared_ptr<FunctionRegistry> function_registry_;
  std::shared_ptr<TypeRegistry> type_registry_;
  std::shared_ptr<Interpreter> interpreter_;

  // All loaded autotuners from all .fal files
  std::map<std::string, atc::AutotunerDecl> loaded_autotuners_;

  // Routine declarations (for validating .so loads)
  std::map<std::string, atc::RoutineDecl> routine_declarations_;
};

} // namespace falcon::autotuner
