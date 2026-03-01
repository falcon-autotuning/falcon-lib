#include "falcon-autotuner/AutotunerEngine.hpp"
#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/StmtExecutor.hpp"
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

  interpreter_ =
      std::make_shared<Interpreter>(function_registry_, type_registry_);
}

bool AutotunerEngine::load_fal_file(const std::string &fal_file_path) {
  try {
    if (!std::filesystem::exists(fal_file_path)) {
      std::ostringstream oss;
      oss << "File not found: " << fal_file_path;
      log::error(oss.str());
      return false;
    }

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
    // Raw pointers into program->structs — the Program is kept alive below.
    for (const auto &struct_decl : program->structs) {
      type_registry_->register_struct(&struct_decl);
    }

    // Process ALL autotuners in the file
    for (auto &autotuner : program->autotuners) {
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

      for (const auto &required : autotuner.required_autotuners) {
        {
          std::ostringstream oss;
          oss << "    requires: " << required;
          log::debug(oss.str());
        }
        if (!has_autotuner(required) &&
            !routine_declarations_.contains(required)) {
          std::ostringstream oss;
          oss << "    Required '" << required
              << "' not yet loaded (must be loaded before running)";
          log::warn(oss.str());
        }
      }

      std::string name = autotuner.name;
      loaded_autotuners_.erase(name);
      loaded_autotuners_.insert({name, std::move(autotuner)});

      auto it = loaded_autotuners_.find(name);
      if (it != loaded_autotuners_.end()) {
        register_autotuner_as_function(it->second);
      }
    }

    // Process ALL routine declarations in the file
    for (auto &routine : program->routines) {
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

      std::string name = routine.name;

      // If the routine has an inline body, register it directly as a callable
      // function now — no .so needed.  This handles routines defined in .fal
      // source files (e.g. `routine Adder (int a, int b) -> (int add) { ... }`)
      // as opposed to routines whose body is compiled into a separate .so.
      if (!routine.body.empty()) {
        register_inline_routine(routine);
      }

      routine_declarations_.erase(name);
      routine_declarations_.insert({name, std::move(routine)});
    }

    // CRITICAL: Keep the Program alive so that raw StructDecl pointers
    // in the TypeRegistry remain valid for the lifetime of this engine.
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

void AutotunerEngine::register_inline_routine(const atc::RoutineDecl &routine) {
  // Capture everything the lambda needs by value/shared_ptr so it remains
  // valid even after the RoutineDecl is moved into routine_declarations_.
  // We capture the routine name and rely on routine_declarations_ (via the
  // engine pointer) to find the body at call time — this is safe because
  // routine_declarations_ entries are never erased after insertion.
  const std::string routine_name = routine.name;

  auto func = [this, routine_name](ParameterMap &inputs) -> FunctionResult {
    // Look up the stored RoutineDecl (guaranteed to exist after load_fal_file).
    auto decl_it = routine_declarations_.find(routine_name);
    if (decl_it == routine_declarations_.end()) {
      throw std::runtime_error("Inline routine not found in declarations: " +
                               routine_name);
    }
    const atc::RoutineDecl &decl = decl_it->second;

    // Build the execution environment: bind all input params by name.
    ParameterMap env;
    for (const auto &param : decl.input_params) {
      auto it = inputs.find(param->name);
      if (it != inputs.end()) {
        env[param->name] = it->second;
      } else if (param->default_value.has_value()) {
        // Default-value evaluation needs an evaluator — bootstrap with empty
        // env; default values must be literals or already-bound names.
        // For now leave unset; StmtExecutor will error if the body reads it.
      }
      // Primitive output params are default-initialized below.
    }

    // Default-initialize output params so the body can assign into them.
    for (const auto &out : decl.output_params) {
      switch (out->type.base_type) {
      case atc::ParamType::Int:
        env[out->name] = int64_t(0);
        break;
      case atc::ParamType::Float:
        env[out->name] = 0.0;
        break;
      case atc::ParamType::Bool:
        env[out->name] = false;
        break;
      case atc::ParamType::String:
        env[out->name] = std::string("");
        break;
      default:
        env[out->name] = nullptr;
        break;
      }
    }

    // Execute the routine body.
    StmtExecutor executor(env, function_registry_, type_registry_);
    executor.execute_block(decl.body);

    // Collect outputs in declaration order.
    FunctionResult result;
    result.reserve(decl.output_params.size());
    for (const auto &out : decl.output_params) {
      auto it = env.find(out->name);
      if (it == env.end()) {
        throw std::runtime_error("Inline routine '" + routine_name +
                                 "' did not set output: " + out->name);
      }
      result.push_back(it->second);
    }
    return result;
  };

  // Build the BuiltinSignature so the FunctionRegistry and call-site
  // argument-matching logic can work with it.
  std::vector<atc::BuiltinSignature::ParamSpec> params;
  std::vector<atc::BuiltinSignature::ParamSpec> returns;
  params.reserve(routine.input_params.size());
  for (const auto &p : routine.input_params) {
    params.emplace_back(p->name, p->type, true);
  }
  returns.reserve(routine.output_params.size());
  for (const auto &p : routine.output_params) {
    returns.emplace_back(p->name, p->type, true);
  }
  atc::BuiltinSignature sig(routine.name, std::move(params),
                            std::move(returns));

  // Register using the same path as external routines so has_function() and
  // the call-site lookup both find it.
  RoutineInfo routine_info{routine.name, /*library_path=*/"<inline>",
                           ExternalFunction(func), sig};
  function_registry_->register_routine(routine_info);

  log::debug(fmt::format("Registered inline routine: {}", routine.name));
}

} // namespace falcon::autotuner
