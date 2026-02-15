#include "falcon-database/DeviceCharacteristic.hpp"
#include <stdexcept>

namespace falcon::database {

// JSONPrimitive implementation
bool JSONPrimitive::as_bool() const {
  if (type_ != Type::Boolean) {
    throw std::runtime_error("JSONPrimitive is not a boolean");
  }
  return bool_value_;
}

int64_t JSONPrimitive::as_int() const {
  if (type_ != Type::Integer) {
    throw std::runtime_error("JSONPrimitive is not an integer");
  }
  return int_value_;
}

double JSONPrimitive::as_double() const {
  if (type_ != Type::Double) {
    throw std::runtime_error("JSONPrimitive is not a double");
  }
  return double_value_;
}

std::string JSONPrimitive::as_string() const {
  if (type_ != Type::String) {
    throw std::runtime_error("JSONPrimitive is not a string");
  }
  return string_value_;
}

json JSONPrimitive::to_json() const {
  switch (type_) {
  case Type::Null:
    return nullptr;
  case Type::Boolean:
    return bool_value_;
  case Type::Integer:
    return int_value_;
  case Type::Double:
    return double_value_;
  case Type::String:
    return string_value_;
  }
  return nullptr;
}

JSONPrimitive JSONPrimitive::from_json(const json &json) {
  if (json.is_null()) {
    return {};
  }
  if (json.is_boolean()) {
    return JSONPrimitive(json.get<bool>());
  }
  if (json.is_number_integer()) {
    return JSONPrimitive(json.get<int64_t>());
  }
  if (json.is_number_float()) {
    return JSONPrimitive(json.get<double>());
  }
  if (json.is_string()) {
    return JSONPrimitive(json.get<std::string>());
  }
  throw std::runtime_error("Unsupported JSON type for JSONPrimitive");
}

// DeviceCharacteristic implementation
json DeviceCharacteristic::to_json() const {
  json json;
  json["scope"] = scope;
  json["name"] = name;

  auto put_opt = [&](const char *key, const auto &opt) {
    if (opt) {
      json[key] = *opt;
    }
  };

  put_opt("barrier_gate", barrier_gate);
  put_opt("plunger_gate", plunger_gate);
  put_opt("reservoir_gate", reservoir_gate);
  put_opt("screening_gate", screening_gate);
  put_opt("extra", extra);
  put_opt("uncertainty", uncertainty);
  put_opt("hash", hash);
  put_opt("recordtime", time);
  put_opt("device_state", state);
  put_opt("unit_name", unit_name);

  json["device_characteristic"] = characteristic.to_json();
  return json;
}

DeviceCharacteristic DeviceCharacteristic::from_json(const json &json) {
  DeviceCharacteristic dchar;
  dchar.scope = json.value("scope", "");
  dchar.name = json.value("name", "");

  auto get_opt_str = [&](const char *key) -> std::optional<std::string> {
    return (json.contains(key) && !json[key].is_null())
               ? std::make_optional(json[key].get<std::string>())
               : std::nullopt;
  };
  auto get_opt_double = [&](const char *key) -> std::optional<double> {
    return (json.contains(key) && !json[key].is_null())
               ? std::make_optional(json[key].get<double>())
               : std::nullopt;
  };
  auto get_opt_int64 = [&](const char *key) -> std::optional<int64_t> {
    return (json.contains(key) && !json[key].is_null())
               ? std::make_optional(json[key].get<int64_t>())
               : std::nullopt;
  };

  dchar.barrier_gate = get_opt_str("barrier_gate");
  dchar.plunger_gate = get_opt_str("plunger_gate");
  dchar.reservoir_gate = get_opt_str("reservoir_gate");
  dchar.screening_gate = get_opt_str("screening_gate");
  dchar.extra = get_opt_str("extra");
  dchar.uncertainty = get_opt_double("uncertainty");
  dchar.hash = get_opt_str("hash");
  dchar.time = get_opt_int64("recordtime");
  dchar.state = get_opt_str("device_state");
  dchar.unit_name = get_opt_str("unit_name");

  if (json.contains("device_characteristic")) {
    dchar.characteristic =
        JSONPrimitive::from_json(json["device_characteristic"]);
  }
  return dchar;
}

} // namespace falcon::database
