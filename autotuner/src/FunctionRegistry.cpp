#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/log.hpp"
#include <falcon-database/DatabaseConnection.hpp>
#include <stdexcept>

namespace falcon::autotuner {

FunctionRegistry::FunctionRegistry()
    : builtin_registry_(atc::BuiltinFunctionRegistry::create_default()) {
  // Builtin registry is initialized with all signatures
}

void FunctionRegistry::register_builtin(const std::string &name,
                                        ExternalFunction func) {

  // Look up signature from the unified builtin registry
  const atc::BuiltinSignature *sig = builtin_registry_.lookup(name);
  if (sig == nullptr) {
    throw std::runtime_error(
        "Cannot register builtin '" + name +
        "': signature not found in BuiltinFunctionRegistry. "
        "Did you forget to add it to BuiltinRegistry.cpp?");
  }

  functions_[name] = std::move(func);
  signatures_[name] = sig;
}

void FunctionRegistry::register_autotuner(const atc::BuiltinSignature *sig,
                                          ExternalFunction func) {
  functions_[sig->qualified_name] = std::move(func);
  signatures_[sig->qualified_name] = sig;
}

void FunctionRegistry::register_routine(const RoutineInfo &routine) {
  functions_[routine.name] = routine.function;
  signatures_[routine.name] = routine.signature;
  routines_[routine.name] = routine;
}

ExternalFunction *FunctionRegistry::lookup(const std::string &name) {
  auto it = functions_.find(name);
  if (it != functions_.end()) {
    return &it->second;
  }
  return nullptr;
}

const atc::BuiltinSignature *
FunctionRegistry::get_signature(const std::string &name) const {
  auto it = signatures_.find(name);
  if (it != signatures_.end()) {
    return it->second;
  }
  return nullptr;
}

const RoutineInfo *
FunctionRegistry::get_routine_info(const std::string &name) const {
  auto it = routines_.find(name);
  if (it != routines_.end()) {
    return &it->second;
  }
  return nullptr;
}

bool FunctionRegistry::has_function(const std::string &name) const {
  return functions_.find(name) != functions_.end();
}

std::shared_ptr<FunctionRegistry> FunctionRegistry::create_default() {
  auto registry = std::make_shared<FunctionRegistry>();
  register_all_builtins(*registry);
  return registry;
}

void register_all_builtins(FunctionRegistry &registry) {

  // ========================================================================
  // DATABASE FUNCTIONS
  // ========================================================================

  // Just provide the name - signature is looked up automatically!
  registry.register_builtin(
      "read", [](const ParameterMap &params) -> FunctionResult {
        std::string scope = std::get<std::string>(params.at("scope"));
        std::string name = std::get<std::string>(params.at("name"));

        database::ReadOnlyDatabaseConnection db;
        database::DeviceCharacteristicQuery query;
        query.scope = scope;
        query.name = name;

        try {
          auto dchars = db.get_by_query(query);

          if (dchars.empty()) {
            // No value found - return (nil, error)
            return FunctionResult{
                {nullptr, ErrorObject{"Value not found", false}}};
          }

          // TODO: Parse actual value from database based on type
          // For now, return dummy value
          RuntimeValue value = static_cast<int64_t>(42);
          RuntimeValue error = nullptr; // nil

          return FunctionResult{value, error};

        } catch (const std::exception &e) {
          // Database error
          return FunctionResult{nullptr, ErrorObject{e.what(), false}};
        }
      });

  registry.register_builtin(
      "write", [](const ParameterMap &params) -> FunctionResult {
        std::string scope = std::get<std::string>(params.at("scope"));
        std::string name = std::get<std::string>(params.at("name"));
        RuntimeValue value = params.at("value");

        database::ReadWriteDatabaseConnection db;
        database::DeviceCharacteristic dchar;
        dchar.scope = scope;
        dchar.name = name;

        // TODO: Convert RuntimeValue to database value

        try {
          db.insert(dchar);
          return FunctionResult{nullptr}; // nil error (success)
        } catch (const std::exception &e) {
          return FunctionResult{ErrorObject{e.what(), false}};
        }
      });

  // ========================================================================
  // LOGGING FUNCTIONS
  // ========================================================================

  registry.register_builtin(
      "logInfo", [](const ParameterMap &params) -> FunctionResult {
        std::string format = std::get<std::string>(params.at("format"));
        log::info(format);
        return FunctionResult{nullptr};
      });

  registry.register_builtin(
      "logWarn", [](const ParameterMap &params) -> FunctionResult {
        std::string format = std::get<std::string>(params.at("format"));
        log::warn(format);
        return FunctionResult{nullptr};
      });

  registry.register_builtin(
      "logError", [](const ParameterMap &params) -> FunctionResult {
        std::string format = std::get<std::string>(params.at("format"));
        log::error(format);
        return FunctionResult{nullptr};
      });

  // ========================================================================
  // ERROR CONSTRUCTION
  // ========================================================================

  registry.register_builtin(
      "errorMsg", [](const ParameterMap &params) -> FunctionResult {
        std::string message = std::get<std::string>(params.at("message"));
        return FunctionResult{ErrorObject{message, false}};
      });

  registry.register_builtin(
      "fatalErrorMsg", [](const ParameterMap &params) -> FunctionResult {
        std::string message = std::get<std::string>(params.at("message"));
        return FunctionResult{ErrorObject{message, true}};
      });
}

} // namespace falcon::autotuner
