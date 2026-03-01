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

struct RoutineConfig {
  std::string name;
  std::string library_path;
  std::string name_space;
};

class AutotunerEngine {
public:
  AutotunerEngine();

  bool load_fal_file(const std::string &fal_file_path);
  bool load_fal_compiled(const std::string &compiled_path);
  bool save_fal_compiled(const std::string &output_path);
  bool load_routine_library(const RoutineConfig &info);
  FunctionResult run_autotuner(const std::string &autotuner_name,
                               ParameterMap &inputs);

  [[nodiscard]] std::vector<std::string> get_loaded_autotuners() const;
  [[nodiscard]] std::vector<std::string> get_loaded_routines() const;
  [[nodiscard]] std::vector<std::string> get_declared_routines() const;
  [[nodiscard]] bool has_autotuner(const std::string &name) const;
  [[nodiscard]] const atc::AutotunerDecl *
  get_autotuner(const std::string &name) const;

  std::shared_ptr<TypeRegistry> get_type_registry() { return type_registry_; }

private:
  void register_autotuner_as_function(const atc::AutotunerDecl &autotuner);

  /// Register a routine that has an inline body (defined in the .fal source)
  /// directly into the function registry so it can be called at runtime
  /// without needing a compiled .so library.
  void register_inline_routine(const atc::RoutineDecl &routine);

  std::shared_ptr<FunctionRegistry> function_registry_;
  std::shared_ptr<TypeRegistry> type_registry_;
  std::shared_ptr<Interpreter> interpreter_;

  std::map<std::string, atc::AutotunerDecl> loaded_autotuners_;
  std::map<std::string, atc::RoutineDecl> routine_declarations_;

  // Parsed Program objects — kept alive so that the StructDecl raw pointers
  // stored in TypeRegistry remain valid for the lifetime of the engine.
  std::vector<std::unique_ptr<atc::Program>> loaded_programs_;
};

} // namespace falcon::autotuner
