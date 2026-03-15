#include "falcon-dsl/FunctionRegistry.hpp"
#include "falcon-dsl/log.hpp"
#include <falcon-database/DatabaseConnection.hpp>
#include <falcon-typing/PrimitiveTypes.hpp>
#include <falcon_core/physics/device_structures/Connection.hpp>
#include <stdexcept>
namespace {
falcon::typing::RuntimeValue json_to_runtime_value(const nlohmann::json &j) {
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
nlohmann::json runtime_value_to_json(const falcon::typing::RuntimeValue &v) {
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
namespace falcon::dsl {

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
  signatures_.emplace(name, *sig);
}

void FunctionRegistry::register_autotuner(const atc::BuiltinSignature &sig,
                                          ExternalFunction func) {
  functions_[sig.qualified_name] = std::move(func);
  signatures_.emplace(sig.qualified_name, sig);
}

void FunctionRegistry::register_routine(const RoutineInfo &routine) {
  functions_[routine.name] = routine.function;
  signatures_.emplace(routine.name, routine.signature);
  routines_.emplace(routine.name, routine);
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
    return &it->second;
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
std::optional<std::string>
get_optional_string(const typing::ParameterMap &params, const char *key) {
  auto it = params.find(key);
  if (it != params.end() && std::holds_alternative<std::string>(it->second)) {
    return std::get<std::string>(it->second);
  }
  return std::nullopt;
}

// // Helper for loading optional Connection fields from ParameterMap
// std::optional<std::string>
// get_optional_connection_name(const ParameterMap &params, const char *key) {
//   auto it = params.find(key);
//   if (it != params.end() &&
//       std::holds_alternative<
//           falcon_core::physics::device_structures::ConnectionSP>(it->second))
//           {
//     auto conn =
//     std::get<falcon_core::physics::device_structures::ConnectionSP>(
//         it->second);
//     return conn->name();
//   }
//   return std::nullopt;
// }
//
// Helper for loading optional double fields from ParameterMap
std::optional<double> get_optional_double(const typing::ParameterMap &params,
                                          const char *key) {
  auto it = params.find(key);
  if (it != params.end() && std::holds_alternative<double>(it->second)) {
    return std::get<double>(it->second);
  }
  return std::nullopt;
}

// Helper for loading optional int64_t fields from ParameterMap
std::optional<int64_t> get_optional_int64(const typing::ParameterMap &params,
                                          const char *key) {
  auto it = params.find(key);
  if (it != params.end() && std::holds_alternative<int64_t>(it->second)) {
    return std::get<int64_t>(it->second);
  }
  return std::nullopt;
}
void register_all_builtins(FunctionRegistry &registry) {

  // ========================================================================
  // ERROR CONSTRUCTION
  // ========================================================================

  registry.register_builtin(
      "errorMsg",
      [](const typing::ParameterMap &params) -> typing::FunctionResult {
        std::string message = std::get<std::string>(params.at("message"));
        return typing::FunctionResult{typing::ErrorObject{message, false}};
      });

  registry.register_builtin(
      "fatalErrorMsg",
      [](const typing::ParameterMap &params) -> typing::FunctionResult {
        std::string message = std::get<std::string>(params.at("message"));
        return typing::FunctionResult{typing::ErrorObject{message, true}};
      });
}

} // namespace falcon::dsl
