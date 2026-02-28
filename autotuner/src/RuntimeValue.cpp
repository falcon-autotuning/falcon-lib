#include "falcon-autotuner/RuntimeValue.hpp"
#include <falcon_core/autotuner_interfaces/names/Gname.hpp>
#include <falcon_core/math/Quantity.hpp>

namespace falcon::autotuner {

std::string get_runtime_type_name(const RuntimeValue &value) {
  if (std::holds_alternative<int64_t>(value)) {
    return "int";
  }
  if (std::holds_alternative<double>(value)) {
    return "float";
  }
  if (std::holds_alternative<bool>(value)) {
    return "bool";
  }
  if (std::holds_alternative<std::string>(value)) {
    return "string";
  }
  if (std::holds_alternative<std::nullptr_t>(value)) {
    return "nil";
  }
  if (std::holds_alternative<ErrorObject>(value)) {
    return "Error";
  }
  if (std::holds_alternative<std::shared_ptr<TupleValue>>(value)) {
    return "tuple";
  }
  if (std::holds_alternative<
          falcon_core::physics::device_structures::ConnectionSP>(value)) {
    return "Connection";
  }
  if (std::holds_alternative<
          falcon_core::physics::device_structures::ConnectionsSP>(value)) {
    return "Connections";
  }
  if (std::holds_alternative<falcon_core::math::QuantitySP>(value)) {
    return "Quantity";
  }
  if (std::holds_alternative<falcon_core::autotuner_interfaces::names::GnameSP>(
          value)) {
    return "Gname";
  }
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(value)) {
    return "StructInstance";
  }
  return "<unknown>";
}

std::string runtime_value_to_string(const RuntimeValue &value) {
  if (std::holds_alternative<int64_t>(value)) {
    return std::to_string(std::get<int64_t>(value));
  }
  if (std::holds_alternative<double>(value)) {
    return std::to_string(std::get<double>(value));
  }
  if (std::holds_alternative<bool>(value)) {
    return std::get<bool>(value) ? "true" : "false";
  }
  if (std::holds_alternative<std::string>(value)) {
    return "\"" + std::get<std::string>(value) + "\"";
  }
  if (std::holds_alternative<std::nullptr_t>(value)) {
    return "nil";
  }
  if (std::holds_alternative<ErrorObject>(value)) {
    const auto &err = std::get<ErrorObject>(value);
    return "Error(" + err.message + ")";
  }
  if (std::holds_alternative<std::shared_ptr<TupleValue>>(value)) {
    const auto &tuplePtr = std::get<std::shared_ptr<TupleValue>>(value);
    if (!tuplePtr)
      return "(nil)";
    std::string result = "(";
    for (size_t i = 0; i < tuplePtr->values.size(); ++i) {
      if (i > 0) {
        result += ", ";
      }
      result += runtime_value_to_string(tuplePtr->values[i]);
    }
    result += ")";
    return result;
  }
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(value)) {
    const auto &structPtr = std::get<std::shared_ptr<StructInstance>>(value);
    if (!structPtr)
      return "<StructInstance:nil>";
    return "<StructInstance:" + structPtr->type_name + ">";
  }
  return "<object:" + get_runtime_type_name(value) + ">";
}

} // namespace falcon::autotuner
