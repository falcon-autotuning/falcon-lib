#include "falcon-autotuner/AutotunerEngine.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/log.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace falcon::autotuner {

AutotunerEngine::AutotunerEngine() {
  function_registry_ = FunctionRegistry::create_default();
  type_registry_ = TypeRegistry::create_default();

  // Interpreter will initialize NATS and config
  interpreter_ =
      std::make_shared<Interpreter>(function_registry_, type_registry_);
}

bool AutotunerEngine::load_fal_file(const std::string &fal_file_path) {
  try {
    // Check if file exists
    if (!std::filesystem::exists(fal_file_path)) {
      std::ostringstream oss;
      oss << "File not found: " << fal_file_path;
      log::error(oss.str());
      return false;
    }

    // Use the compiler from autotuner/compiler/
    atc::Compiler compiler;

    auto program = compiler.parse_file(fal_file_path);
    if (!program) {
      std::ostringstream oss;
      oss << "Failed to parse: " << fal_file_path;
      log::error(oss.str());

      return false;
    }

    {
      std::ostringstream oss;
      oss << "Loaded .fal file: " << fal_file_path;
      log::info(oss.str());
    }

    {
      std::ostringstream oss;
      oss << "Found " << program->autotuners.size() << " autotuner(s), "
          << program->routines.size() << " routine(s)";
      log::info(oss.str());
    }

    // Process ALL autotuners in the file
    for (auto &autotuner : program->autotuners) {
      // Check for duplicate names
      if (loaded_autotuners_.find(autotuner.name) != loaded_autotuners_.end()) {
        std::ostringstream oss;
        oss << "Autotuner '" << autotuner.name
            << "' already loaded, overwriting";
        log::warn(oss.str());
      }

      {
        std::ostringstream oss;
        oss << "  - Autotuner: " << autotuner.name;
        log::info(oss.str());
      }

      // Validate dependencies
      for (const auto &required : autotuner.required_autotuners) {
        {
          std::ostringstream oss;
          oss << "    requires: " << required;
          log::debug(oss.str());
        }

        // Check if required autotuner/routine exists or will be loaded
        if (!has_autotuner(required) && routine_declarations_.find(required) ==
                                            routine_declarations_.end()) {
          std::ostringstream oss;
          oss << "    Required '" << required
              << "' not yet loaded (must be loaded before running)";
          log::warn(oss.str());
        }
      }

      // Store the autotuner (move it since AutotunerDecl is move-only)
      std::string name = autotuner.name;
      loaded_autotuners_[name] = std::move(autotuner);

      // Register as callable function
      register_autotuner_as_function(loaded_autotuners_[name]);
    }

    // Process ALL routine declarations in the file
    for (auto &routine : program->routines) {
      // Check for duplicate names
      if (routine_declarations_.find(routine.name) !=
          routine_declarations_.end()) {
        std::ostringstream oss;
        oss << "Routine '" << routine.name << "' already declared, overwriting";
        log::warn(oss.str());
      }

      {
        std::ostringstream oss;
        oss << "  - Routine: " << routine.name;
        log::info(oss.str());
      }

      // Store routine declaration (for later .so loading)
      std::string name = routine.name;
      routine_declarations_[name] = std::move(routine);
    }

    return true;
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Error loading file: " << e.what();
    log::error(oss.str());
    return false;
  }
}

bool AutotunerEngine::load_fal_compiled(const std::string &compiled_path) {
  // Load pre-compiled .fal file (serialized AST)
  try {
    std::ifstream file(compiled_path, std::ios::binary);
    if (!file.is_open()) {
      std::ostringstream oss;
      oss << "Failed to open compiled file: " << compiled_path;
      log::error(oss.str());
      return false;
    }

    // TODO: Implement AST serialization/deserialization
    log::warn("Compiled .fal loading not yet implemented, use load_fal_file()");
    return false;
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Error loading compiled file: " << e.what();
    log::error(oss.str());
    return false;
  }
}

bool AutotunerEngine::save_fal_compiled(const std::string &output_path) {
  // Save all loaded autotuners as pre-compiled binary
  try {
    std::ofstream file(output_path, std::ios::binary);
    if (!file.is_open()) {
      std::ostringstream oss;
      oss << "Failed to create compiled file: " << output_path;
      log::error(oss.str());
      return false;
    }

    // TODO: Implement AST serialization
    log::warn("Compiled .fal saving not yet implemented");
    return false;
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Error saving compiled file: " << e.what();
    log::error(oss.str());
    return false;
  }
}

bool AutotunerEngine::load_routine_library(const std::string &routine_name,
                                           const std::string &namespace_name,
                                           const std::string &library_path) {
  // Check if routine was declared
  auto it = routine_declarations_.find(routine_name);
  if (it == routine_declarations_.end()) {
    std::ostringstream oss;
    oss << "Routine '" << routine_name
        << "' not declared in any loaded .fal file";
    log::error(oss.str());

    // Build list of available routines
    std::ostringstream list_oss;
    list_oss << "Available routines: ";
    for (const auto &r : routine_declarations_) {
      list_oss << r.first << " ";
    }
    log::info(list_oss.str());
    return false;
  }

  const atc::RoutineDecl &routine_decl = it->second;

  try {
    function_registry_->load_routine(routine_name, namespace_name, library_path,
                                     routine_decl.input_params,
                                     routine_decl.output_params);

    std::ostringstream oss;
    oss << "Loaded routine: " << routine_name << " from " << library_path;
    log::info(oss.str());
    return true;
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Failed to load routine: " << e.what();
    log::error(oss.str());
    return false;
  }
}

ParameterMap AutotunerEngine::run_autotuner(const std::string &autotuner_name,
                                            const ParameterMap &inputs) {
  auto it = loaded_autotuners_.find(autotuner_name);
  if (it == loaded_autotuners_.end()) {
    throw std::runtime_error("Autotuner not loaded: " + autotuner_name);
  }

  // Validate that all required dependencies are available
  const auto &autotuner = it->second;
  for (const auto &required : autotuner.required_autotuners) {
    if (!function_registry_->has_simple(required)) {
      throw std::runtime_error("Required dependency '" + required +
                               "' not loaded for autotuner '" + autotuner_name +
                               "'");
    }
  }

  std::ostringstream oss;
  oss << "Running autotuner: " << autotuner_name;
  log::info(oss.str());

  return interpreter_->run(autotuner, inputs);
}

std::vector<std::string> AutotunerEngine::get_loaded_autotuners() const {
  std::vector<std::string> names;
  names.reserve(loaded_autotuners_.size());
  for (const auto &pair : loaded_autotuners_) {
    names.push_back(pair.first);
  }
  return names;
}

std::vector<std::string> AutotunerEngine::get_loaded_routines() const {
  std::vector<std::string> names;
  for (const auto &pair : routine_declarations_) {
    // Check if the routine has been loaded from .so
    if (function_registry_->has_simple(pair.first)) {
      names.push_back(pair.first);
    }
  }
  return names;
}

std::vector<std::string> AutotunerEngine::get_declared_routines() const {
  std::vector<std::string> names;
  names.reserve(routine_declarations_.size());
  for (const auto &pair : routine_declarations_) {
    names.push_back(pair.first);
  }
  return names;
}

bool AutotunerEngine::has_autotuner(const std::string &name) const {
  return loaded_autotuners_.find(name) != loaded_autotuners_.end();
}

const atc::AutotunerDecl *
AutotunerEngine::get_autotuner(const std::string &name) const {
  auto it = loaded_autotuners_.find(name);
  return it != loaded_autotuners_.end() ? &it->second : nullptr;
}

void AutotunerEngine::register_autotuner_as_function(
    const atc::AutotunerDecl &autotuner) {
  // Wrap autotuner in callable function
  auto func = [this, name = autotuner.name](
                  const ParameterMap &inputs) -> ParameterMap {
    return interpreter_->run(loaded_autotuners_[name], inputs);
  };

  function_registry_->register_autotuner(autotuner.name, func);
}

} // namespace falcon::autotuner
