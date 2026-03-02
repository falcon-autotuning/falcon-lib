#pragma once

#include "falcon-atc/AST.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/TypeRegistry.hpp"
#include <filesystem>
#include <map>
#include <memory>
#include <set>
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
  FunctionResult run_routine(const std::string &routine_name,
                             ParameterMap &inputs);

  [[nodiscard]] std::vector<std::string> get_loaded_autotuners() const;
  [[nodiscard]] std::vector<std::string> get_loaded_routines() const;
  [[nodiscard]] std::vector<std::string> get_declared_routines() const;
  [[nodiscard]] bool has_autotuner(const std::string &name) const;
  [[nodiscard]] const atc::AutotunerDecl *
  get_autotuner(const std::string &name) const;

  std::shared_ptr<TypeRegistry> get_type_registry() { return type_registry_; }

private:
  bool process_ff_import(const atc::FFImportDecl &ffi,
                         const std::filesystem::path &fal_dir,
                         const atc::Program &program);
  void register_autotuner_as_function(const atc::AutotunerDecl &autotuner);
  void register_inline_routine(const atc::RoutineDecl &routine);
  // Helper to compute a SHA-256 (or simpler) hash of a file for cache keys
  static std::string compute_file_hash(const std::filesystem::path &path);

  /// Lightweight: extract raw import path strings without full parsing
  static std::vector<std::string>
  extract_imports_from_file(const std::filesystem::path &path);

  std::shared_ptr<FunctionRegistry> function_registry_;
  std::shared_ptr<TypeRegistry> type_registry_;
  std::shared_ptr<Interpreter> interpreter_;

  std::map<std::string, atc::AutotunerDecl> loaded_autotuners_;
  std::map<std::string, atc::RoutineDecl> routine_declarations_;

  // Lifetime storage for parsed Programs (raw StructDecl* pointers in
  // TypeRegistry point into these).
  std::vector<std::unique_ptr<atc::Program>> loaded_programs_;

  // Track which absolute paths have already been loaded (cycle prevention)
  std::set<std::filesystem::path> loaded_paths_;

  // Map abs_path → raw Program* for cross-file struct name lookup
  std::map<std::filesystem::path, const atc::Program *>
      loaded_programs_by_path_;

  // Struct type names imported from other modules; passed to the Compiler
  // as hints so the parser accepts them in type_spec positions.
  std::set<std::string> import_struct_hints_;
  std::vector<void *> ff_handles_; // dlopen handles — kept open for lifetime
  // Holds dlopen handles for FFI shared libraries so they stay loaded
  // for the lifetime of the engine.
  std::vector<void *> ffi_handles_;
};

} // namespace falcon::autotuner
