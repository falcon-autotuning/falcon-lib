#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/TypeRegistry.hpp"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace falcon::autotuner {

/**
 * @brief Configuration for loading a routine library.
 */
struct RoutineConfig {
  std::string name;
  std::string library_path;
  std::string name_space;
};

/**
 * @brief The top-level engine that orchestrates autotuner loading and
 * execution.
 *
 * Owns:
 * - FunctionRegistry  — all callable functions (builtins + loaded routines)
 * - TypeRegistry      — type methods + user-defined struct declarations
 * - Interpreter       — executes loaded autotuners
 * - loaded_autotuners_ — parsed AutotunerDecl objects (move-only, stored by
 *   name)
 * - loaded_programs_  — parsed Program objects; kept alive so that
 *   StructDecl pointers registered in the TypeRegistry remain valid
 */
class AutotunerEngine {
public:
  AutotunerEngine();

  /**
   * @brief Parse and load a .fal source file.
   * Registers all struct types, autotuners, and routine declarations found.
   */
  bool load_fal_file(const std::string &fal_file_path);

  /**
   * @brief Load a pre-compiled (serialized) .fal file. (Not yet implemented.)
   */
  bool load_fal_compiled(const std::string &compiled_path);

  /**
   * @brief Serialize currently loaded autotuners. (Not yet implemented.)
   */
  bool save_fal_compiled(const std::string &output_path);

  /**
   * @brief Load a compiled routine .so and register it for execution.
   */
  bool load_routine_library(const RoutineConfig &info);

  /**
   * @brief Execute a previously loaded autotuner by name.
   */
  FunctionResult run_autotuner(const std::string &autotuner_name,
                               ParameterMap &inputs);

  /**
   * @brief List names of all loaded (runnable) autotuners.
   */
  [[nodiscard]] std::vector<std::string> get_loaded_autotuners() const;

  /**
   * @brief List names of routines that have been both declared and loaded.
   */
  [[nodiscard]] std::vector<std::string> get_loaded_routines() const;

  /**
   * @brief List names of all declared routines (whether or not the .so is
   * loaded).
   */
  [[nodiscard]] std::vector<std::string> get_declared_routines() const;

  /**
   * @brief Check whether an autotuner with the given name is loaded.
   */
  [[nodiscard]] bool has_autotuner(const std::string &name) const;

  /**
   * @brief Return a pointer to the AutotunerDecl for the given name, or
   * nullptr.
   */
  [[nodiscard]] const atc::AutotunerDecl *
  get_autotuner(const std::string &name) const;

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

  // Parsed Program objects — kept alive so that the StructDecl raw pointers
  // stored in TypeRegistry remain valid for the lifetime of the engine.
  // IMPORTANT: must be declared AFTER type_registry_ so it is destroyed
  // BEFORE type_registry_ (though in practice the registry only reads them).
  std::vector<std::unique_ptr<atc::Program>> loaded_programs_;
};

} // namespace falcon::autotuner
