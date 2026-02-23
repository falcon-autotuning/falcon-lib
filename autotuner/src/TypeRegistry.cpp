#include "falcon-autotuner/TypeRegistry.hpp"
#include <falcon_core/math/Quantity.hpp>
#include <falcon_core/physics/device_structures/Connection.hpp>
#include <iostream>

namespace falcon::autotuner {

void TypeRegistry::register_method(const std::string &type_name,
                                   const std::string &method_name,
                                   MethodFunction func) {
  type_methods_[type_name][method_name] = std::move(func);
}

TypeRegistry::MethodFunction *
TypeRegistry::lookup_method(const std::string &type_name,
                            const std::string &method_name) {
  auto type_it = type_methods_.find(type_name);
  if (type_it == type_methods_.end()) {
    return nullptr;
  }

  auto method_it = type_it->second.find(method_name);
  if (method_it == type_it->second.end()) {
    return nullptr;
  }

  return &method_it->second;
}

bool TypeRegistry::has_method(const std::string &type_name,
                              const std::string &method_name) const {
  auto type_it = type_methods_.find(type_name);
  if (type_it == type_methods_.end()) {
    return false;
  }
  return type_it->second.find(method_name) != type_it->second.end();
}

std::shared_ptr<TypeRegistry> TypeRegistry::create_default() {
  auto registry = std::make_shared<TypeRegistry>();
  register_all_type_methods(*registry);
  return registry;
}

// ============================================================================
// BUILTIN TYPE METHOD IMPLEMENTATIONS
// ============================================================================

void register_all_type_methods(TypeRegistry &registry) {

  // ------------------------------------------------------------------------
  // Connection methods
  // ------------------------------------------------------------------------

  registry.register_method(
      "Connection", "name",
      [](const RuntimeValue &obj, const ParameterMap &params) -> ParameterMap {
        auto &conn =
            std::get<falcon_core::physics::device_structures::ConnectionSP>(
                obj);
        // TODO: Call actual falcon-core method
        std::string name = "connection_stub"; // conn.wrapped_object->name();
        return {{"result", name}};
      });

  registry.register_method(
      "Connection", "set_voltage",
      [](const RuntimeValue &obj, const ParameterMap &params) -> ParameterMap {
        auto &conn =
            std::get<falcon_core::physics::device_structures::ConnectionSP>(
                obj);
        double voltage = std::get<double>(params.at("voltage"));
        // TODO: conn.wrapped_object->set_voltage(voltage);
        std::cout << "[STUB] Connection::set_voltage(" << voltage << ")"
                  << '\n';
        return {}; // void return
      });

  // ------------------------------------------------------------------------
  // Connections methods
  // ------------------------------------------------------------------------

  registry.register_method(
      "Connections", "size",
      [](const RuntimeValue &obj, const ParameterMap &params) -> ParameterMap {
        auto &conns =
            std::get<falcon_core::physics::device_structures::ConnectionsSP>(
                obj);
        // TODO: int64_t size = conns.wrapped_object->size();
        int64_t size = 5; // Stub
        return {{"result", size}};
      });

  // ------------------------------------------------------------------------
  // Quantity methods
  // ------------------------------------------------------------------------

  registry.register_method(
      "Quantity", "value",
      [](const RuntimeValue &obj, const ParameterMap &params) -> ParameterMap {
        auto &qty = std::get<falcon_core::math::QuantitySP>(obj);
        // TODO: double value = qty.wrapped_object->value();
        double value = 3.14; // Stub
        return {{"result", value}};
      });

  registry.register_method(
      "Quantity", "unit",
      [](const RuntimeValue &obj, const ParameterMap &params) -> ParameterMap {
        auto &qty = std::get<falcon_core::math::QuantitySP>(obj);
        // TODO: std::string unit = qty.wrapped_object->unit();
        std::string unit = "V"; // Stub
        return {{"result", unit}};
      });

  // Add more type methods as needed...
}

} // namespace falcon::autotuner
