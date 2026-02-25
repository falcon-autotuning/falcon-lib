#include "falcon-autotuner/FunctionRegistry.hpp"
#include "falcon-autotuner/RuntimeValue.hpp"
#include "falcon-autotuner/log.hpp"
#include <falcon-database/DatabaseConnection.hpp>
#include <stdexcept>
namespace {
falcon::autotuner::RuntimeValue json_to_runtime_value(const nlohmann::json &j) {
  if (j.is_null()) {
    return nullptr;
  }
  if (j.is_boolean()) {
    return j.get<bool>();
  }
  if (j.is_number_integer()) {
    return static_cast<int64_t>(j.get<int64_t>());
  }
  if (j.is_number_float()) {
    return j.get<double>();
  }
  if (j.is_string()) {
    return j.get<std::string>();
  }
  throw std::runtime_error("Unsupported JSON type for RuntimeValue");
}
nlohmann::json runtime_value_to_json(const falcon::autotuner::RuntimeValue &v) {
  return std::visit(
      [](auto &&arg) -> nlohmann::json {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t> || std::is_same_v<T, double> ||
                      std::is_same_v<T, bool> ||
                      std::is_same_v<T, std::string> ||
                      std::is_same_v<T, std::nullptr_t>) {
          return arg;
        } else {
          throw std::runtime_error(
              "Unsupported RuntimeValue type for JSON conversion");
        }
      },
      v);
}
} // namespace
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

// Helper for loading optional string fields from ParameterMap
void load_optional_string(const ParameterMap &params, const char *key,
                          std::optional<std::string> &field) {
  auto it = params.find(key);
  if (it != params.end())
    field = std::get<std::string>(it->second);
}

// Helper for loading optional double fields from ParameterMap
void load_optional_double(const ParameterMap &params, const char *key,
                          std::optional<double> &field) {
  auto it = params.find(key);
  if (it != params.end())
    field = std::get<double>(it->second);
}

// Helper for loading optional int64_t fields from ParameterMap
void load_optional_int64(const ParameterMap &params, const char *key,
                         std::optional<int64_t> &field) {
  auto it = params.find(key);
  if (it != params.end())
    field = std::get<int64_t>(it->second);
}
void register_all_builtins(FunctionRegistry &registry) {

  // ========================================================================
  // DATABASE FUNCTIONS
  // ========================================================================

  registry.register_builtin(
      "readLatest", [](const ParameterMap &params) -> FunctionResult {
        std::string scope = std::get<std::string>(params.at("scope"));
        std::string name = std::get<std::string>(params.at("name"));
        database::ReadOnlyDatabaseConnection db;
        database::DeviceCharacteristicQuery query;
        query.scope = scope;
        query.name = name;

        load_optional_string(params, "barrier_gate", query.barrier_gate);
        load_optional_string(params, "plunger_gate", query.plunger_gate);
        load_optional_string(params, "reservoir_gate", query.reservoir_gate);
        load_optional_string(params, "screening_gate", query.screening_gate);
        load_optional_string(params, "extra", query.extra);
        load_optional_double(params, "uncertainty", query.uncertainty);
        load_optional_string(params, "hash", query.hash);
        load_optional_int64(params, "time", query.time);
        load_optional_string(params, "state", query.state);
        load_optional_string(params, "unit_name", query.unit_name);

        try {
          auto dchars = db.get_by_query(query);
          if (dchars.empty()) {
            // No value found - return (nil, error)
            return FunctionResult{
                {nullptr, ErrorObject{"Value not found", false}}};
          }

          // Find the DeviceCharacteristic with the highest time, if any have
          // time
          auto best_it = std::max_element(
              dchars.begin(), dchars.end(),
              [](const database::DeviceCharacteristic &a,
                 const database::DeviceCharacteristic &b) {
                if (a.time && b.time) {
                  return *a.time < *b.time;
                }
                if (a.time) {
                  return false; // a has time, b does not
                }
                if (b.time) {
                  return true; // b has time, a does not
                }
                return false; // neither has time, keep original order
              });

          const database::DeviceCharacteristic &chosen =
              (best_it != dchars.end() && best_it->time) ? *best_it
                                                         : dchars.front();

          RuntimeValue value = json_to_runtime_value(chosen.characteristic);
          RuntimeValue error = nullptr;

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
        RuntimeValue value = params.at("characteristic");
        database::ReadWriteDatabaseConnection db;
        database::DeviceCharacteristic dchar;
        dchar.scope = scope;
        dchar.name = name;
        dchar.characteristic = runtime_value_to_json(value);

        // Optionally fill out other fields from params if needed
        load_optional_string(params, "barrier_gate", dchar.barrier_gate);
        load_optional_string(params, "plunger_gate", dchar.plunger_gate);
        load_optional_string(params, "reservoir_gate", dchar.reservoir_gate);
        load_optional_string(params, "screening_gate", dchar.screening_gate);
        load_optional_string(params, "extra", dchar.extra);
        load_optional_double(params, "uncertainty", dchar.uncertainty);
        load_optional_string(params, "hash", dchar.hash);
        load_optional_int64(params, "time", dchar.time);
        load_optional_string(params, "state", dchar.state);
        load_optional_string(params, "unit_name", dchar.unit_name);

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
        std::string format = std::get<std::string>(params.at("arg1"));
        log::info(format);
        return FunctionResult{nullptr};
      });

  registry.register_builtin(
      "logWarn", [](const ParameterMap &params) -> FunctionResult {
        std::string format = std::get<std::string>(params.at("arg1"));
        log::warn(format);
        return FunctionResult{nullptr};
      });

  registry.register_builtin(
      "logError", [](const ParameterMap &params) -> FunctionResult {
        std::string format = std::get<std::string>(params.at("arg1"));
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
