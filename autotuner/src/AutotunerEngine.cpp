#include "falcon-autotuner/AutotunerEngine.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/log.hpp"
#include <dlfcn.h>
#include <filesystem>
#include <fmt/format.h>
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

    // Register user-defined structs in the type registry.
    // We store raw pointers into program->structs here, so the Program object
    // MUST outlive these pointers.  We guarantee that by moving `program` into
    // loaded_programs_ below BEFORE returning from this function.
    for (const auto &struct_decl : program->structs) {
      type_registry_->register_struct(&struct_decl);
    }

    // Process ALL autotuners in the file
    for (auto &autotuner : program->autotuners) {
      // Check for duplicate names
      if (loaded_autotuners_.contains(autotuner.name)) {
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
        if (!has_autotuner(required) &&
            !routine_declarations_.contains(required)) {
          std::ostringstream oss;
          oss << "    Required '" << required
              << "' not yet loaded (must be loaded before running)";
          log::warn(oss.str());
        }
      }

      // Store the autotuner (move it since AutotunerDecl is move-only)
      std::string name = autotuner.name;
      loaded_autotuners_.erase(name);
      loaded_autotuners_.insert({name, std::move(autotuner)});

      // Register as callable function
      auto it = loaded_autotuners_.find(name);
      if (it != loaded_autotuners_.end()) {
        register_autotuner_as_function(it->second);
      }
    }

    // Process ALL routine declarations in the file
    for (auto &routine : program->routines) {
      // Check for duplicate names
      if (routine_declarations_.contains(routine.name)) {
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
      routine_declarations_.erase(name);
      routine_declarations_.insert({name, std::move(routine)});
    }

    // CRITICAL: Keep the Program alive so that the raw StructDecl pointers
    // registered above in type_registry_ remain valid for the lifetime of
    // this engine.  Without this the StructDecl objects are destroyed when
    // `program` goes out of scope at the end of this function, leaving
    // dangling pointers in the TypeRegistry.
    loaded_programs_.push_back(std::move(program));

    return true;
  } catch (const std::exception &e) {
    std::ostringstream oss;
    oss << "Error loading file: " << e.what();
    log::error(oss.str());
    return false;
  }
}

bool AutotunerEngine::load_fal_compiled(const std::string &compiled_path) {
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

bool AutotunerEngine::load_routine_library(const RoutineConfig &info) {
  // Check if routine was declared
  auto decl_it = routine_declarations_.find(info.name);
  if (decl_it == routine_declarations_.end()) {
    std::ostringstream oss;
    oss << "Routine '" << info.name << "' not declared in any loaded .fal file";
    log::error(oss.str());
    std::ostringstream list_oss;
    list_oss << "Available routines: ";
    for (const auto &r : routine_declarations_) {
      list_oss << r.first << " ";
    }
    log::info(list_oss.str());
    return false;
  }

  // Load the .so file
  void *handle = dlopen(info.library_path.c_str(), RTLD_LAZY);
  if (handle == nullptr) {
    std::ostringstream oss;
    oss << "Failed to load library: " << info.library_path << " (" << dlerror()
        << ")";
    log::error(oss.str());
    return false;
  }

  std::string symbol =
      info.name_space.empty() ? info.name : info.name_space + "::" + info.name;

  dlerror();

  void *sym = dlsym(handle, symbol.c_str());
  const char *dlsym_error = dlerror();
  if (dlsym_error != nullptr) { /* error handling */
  }

  auto func_ptr = reinterpret_cast<FunctionResult (*)(ParameterMap &)>(sym);

  ExternalFunction ext_func = [func_ptr](ParameterMap &params) {
    return func_ptr(params);
  };

  const auto &decl = decl_it->second;
  std::vector<atc::BuiltinSignature::ParamSpec> params;
  std::vector<atc::BuiltinSignature::ParamSpec> returns;
  params.reserve(decl.input_params.size());
  for (const auto &p : decl.input_params) {
    params.emplace_back(p->name, p->type, true);
  }
  returns.reserve(decl.output_params.size());
  for (const auto &p : decl.output_params) {
    returns.emplace_back(p->name, p->type, true);
  }
  atc::BuiltinSignature sig(decl.name, std::move(params), std::move(returns));

  RoutineInfo routine_info{info.name, info.library_path, ext_func, sig};
  function_registry_->register_routine(routine_info);

  log::debug(fmt::format("Loaded routine: {} from {} (symbol: {})", info.name,
                         info.library_path, symbol));
  return true;
}

FunctionResult AutotunerEngine::run_autotuner(const std::string &autotuner_name,
                                              ParameterMap &inputs) {
  auto it = loaded_autotuners_.find(autotuner_name);
  if (it == loaded_autotuners_.end()) {
    throw std::runtime_error("Autotuner not loaded: " + autotuner_name);
  }

  const auto &autotuner = it->second;
  for (const auto &required : autotuner.required_autotuners) {
    if (!function_registry_->has_function(required)) {
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
    if (function_registry_->has_function(pair.first)) {
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
  auto iter = loaded_autotuners_.find(name);
  return iter != loaded_autotuners_.end() ? &iter->second : nullptr;
}

void AutotunerEngine::register_autotuner_as_function(
    const atc::AutotunerDecl &autotuner) {
  auto func = [this,
               name = autotuner.name](ParameterMap &inputs) -> FunctionResult {
    auto iter = loaded_autotuners_.find(name);
    return interpreter_->run(iter->second, inputs);
  };

  std::vector<atc::BuiltinSignature::ParamSpec> params;
  std::vector<atc::BuiltinSignature::ParamSpec> returns;
  params.reserve(autotuner.input_params.size());
  for (const auto &param : autotuner.input_params) {
    params.emplace_back(param->name, param->type, true);
  }
  returns.reserve(autotuner.output_params.size());
  for (const auto &param : autotuner.output_params) {
    returns.emplace_back(param->name, param->type, true);
  }
  const atc::BuiltinSignature sig(autotuner.name, std::move(params),
                                  std::move(returns));

  function_registry_->register_autotuner(sig, func);

  log::debug(fmt::format("Loaded autotuner: {}", autotuner.name));
}

} // namespace falcon::autotuner
