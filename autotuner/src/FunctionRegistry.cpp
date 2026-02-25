#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/log.hpp"
#include <falcon-database/DatabaseConnection.hpp>
#include <iostream>
#include <stdexcept>

namespace falcon::autotuner {

FunctionRegistry::FunctionRegistry() {}

FunctionRegistry::~FunctionRegistry() {
  // Clean up loaded libraries
  for (void *handle : loaded_libraries_) {
    if (handle) {
      dlclose(handle);
    }
  }
}

void FunctionRegistry::register_builtin(const std::string &qualified_name,
                                        ExternalFunction func) {
  builtin_functions_[qualified_name] = std::move(func);
}

void FunctionRegistry::load_routine(
    const std::string &routine_name, const std::string &namespace_name,
    const std::string &library_path,
    const std::vector<std::unique_ptr<atc::ParamDecl>> &input_params,
    const std::vector<std::unique_ptr<atc::ParamDecl>> &output_params) {
  // Load the shared library
  void *handle = dlopen(library_path.c_str(), RTLD_LAZY);
  if (handle == nullptr) {
    throw std::runtime_error("Failed to load routine library: " +
                             std::string(dlerror()));
  }

  // Construct the symbol name: <namespace>::<routine_name>
  // C++ name mangling is avoided by using extern "C" in the .so
  std::string symbol_name = namespace_name + "_" + routine_name;

  // Look up the function
  using RoutineFuncPtr = ParameterMap (*)(const ParameterMap &);
  auto func_ptr =
      reinterpret_cast<RoutineFuncPtr>(dlsym(handle, symbol_name.c_str()));

  if (func_ptr == nullptr) {
    dlclose(handle);
    throw std::runtime_error("Failed to find routine symbol: " + symbol_name +
                             " - " + std::string(dlerror()));
  }

  // Wrap in std::function
  ExternalFunction func =
      [func_ptr](const ParameterMap &params) -> ParameterMap {
    return func_ptr(params);
  };

  // Store routine info
  RoutineInfo info;
  info.name = routine_name;
  info.input_params.clear();
  for (const auto &param : input_params) {
    info.input_params.push_back(param->clone());
  }
  info.output_params.clear();
  for (const auto &param : input_params) {
    info.output_params.push_back(param->clone());
  }
  info.library_handle = handle;
  info.function = std::move(func);

  user_routines_[routine_name] = std::move(info);
  loaded_libraries_.push_back(handle);

  std::cout << "[FunctionRegistry] Loaded routine: " << routine_name << " from "
            << library_path << '\n';
}

void FunctionRegistry::register_autotuner(const std::string &name,
                                          ExternalFunction func) {
  autotuner_functions_[name] = std::move(func);
}

ExternalFunction *
FunctionRegistry::lookup_qualified(const std::string &qualified_name) {
  auto it = builtin_functions_.find(qualified_name);
  return it != builtin_functions_.end() ? &it->second : nullptr;
}

ExternalFunction *FunctionRegistry::lookup_simple(const std::string &name) {
  // First check user routines
  auto routine_it = user_routines_.find(name);
  if (routine_it != user_routines_.end()) {
    return &routine_it->second.function;
  }

  // Then check autotuners
  auto autotuner_it = autotuner_functions_.find(name);
  if (autotuner_it != autotuner_functions_.end()) {
    return &autotuner_it->second;
  }

  return nullptr;
}

const RoutineInfo *
FunctionRegistry::get_routine_info(const std::string &name) const {
  auto it = user_routines_.find(name);
  return it != user_routines_.end() ? &it->second : nullptr;
}

bool FunctionRegistry::has_qualified(const std::string &qualified_name) const {
  return builtin_functions_.find(qualified_name) != builtin_functions_.end();
}

bool FunctionRegistry::has_simple(const std::string &name) const {
  return user_routines_.find(name) != user_routines_.end() ||
         autotuner_functions_.find(name) != autotuner_functions_.end();
}

std::shared_ptr<FunctionRegistry> FunctionRegistry::create_default() {
  auto registry = std::make_shared<FunctionRegistry>();
  register_all_builtins(*registry);
  return registry;
}

// ============================================================================
// BUILTIN FUNCTION IMPLEMENTATIONS (Hardcoded in Falcon)
// ============================================================================

void register_all_builtins(FunctionRegistry &registry) {

  // ------------------------------------------------------------------------
  // Logging functions (log::*)
  // ------------------------------------------------------------------------

  registry.register_builtin(
      "log::info", [](const ParameterMap &params) -> ParameterMap {
        // Format string support with {} placeholders
        std::string format = std::get<std::string>(params.at("format"));
        log::info(format);
        return {};
      });

  registry.register_builtin(
      "log::warn", [](const ParameterMap &params) -> ParameterMap {
        std::string format = std::get<std::string>(params.at("format"));
        log::warn(format);
        return {};
      });

  registry.register_builtin(
      "log::error", [](const ParameterMap &params) -> ParameterMap {
        std::string format = std::get<std::string>(params.at("format"));
        log::error(format);
        return {};
      });

  // ------------------------------------------------------------------------
  // Error constructors (Error::*, FatalError::*)
  // ------------------------------------------------------------------------

  registry.register_builtin(
      "Error::msg", [](const ParameterMap &params) -> ParameterMap {
        std::string msg = std::get<std::string>(params.at("message"));

        ErrorObject err;
        err.message = msg;
        err.is_fatal = false;

        return {{"result", err}};
      });

  registry.register_builtin(
      "FatalError::msg", [](const ParameterMap &params) -> ParameterMap {
        std::string msg = std::get<std::string>(params.at("message"));

        ErrorObject err;
        err.message = msg;
        err.is_fatal = true;

        return {{"result", err}};
      });

  // ------------------------------------------------------------------------
  // Database functions
  // ------------------------------------------------------------------------

  registry.register_builtin(
      "read", [](const ParameterMap &params) -> ParameterMap {
        std::string scope = std::get<std::string>(params.at("scope"));
        std::string name = std::get<std::string>(params.at("name"));
        database::ReadOnlyDatabaseConnection db;
        database::DeviceCharacteristicQuery query;
        query.scope = scope;
        query.name = name;
        auto dchars = db.get_by_query(query);

        // Return (value, error) tuple
        return {
            {"value", static_cast<int64_t>(42)}, {"error", nullptr} // nil
        };
      });

  registry.register_builtin(
      "write", [](const ParameterMap &params) -> ParameterMap {
        std::string scope = std::get<std::string>(params.at("scope"));
        std::string name = std::get<std::string>(params.at("name"));

        std::cout << "[STUB] write(scope=" << scope << ", name=" << name << ")"
                  << std::endl;
        return {};
      });

  // Add more builtin functions as needed...
}

} // namespace falcon::autotuner
