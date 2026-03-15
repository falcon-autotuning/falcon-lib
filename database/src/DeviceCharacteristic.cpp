#include "falcon-database/DeviceCharacteristic.hpp"
#include <iostream>
namespace falcon::database {

bool DeviceCharacteristicQuery::operator==(
    const DeviceCharacteristicQuery &other) const {
  return scope == other.scope && name == other.name &&
         barrier_gate == other.barrier_gate &&
         plunger_gate == other.plunger_gate &&
         reservoir_gate == other.reservoir_gate &&
         screening_gate == other.screening_gate && extra == other.extra &&
         uncertainty == other.uncertainty && hash == other.hash &&
         time == other.time && state == other.state &&
         unit_name == other.unit_name;
}

bool DeviceCharacteristicQuery::operator!=(
    const DeviceCharacteristicQuery &other) const {
  return !(*this == other);
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
  json["characteristic"] = characteristic;
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

  dchar.characteristic = json["characteristic"];
  return dchar;
}

bool DeviceCharacteristic::operator==(const DeviceCharacteristic &other) const {
  return scope == other.scope && name == other.name &&
         barrier_gate == other.barrier_gate &&
         plunger_gate == other.plunger_gate &&
         reservoir_gate == other.reservoir_gate &&
         screening_gate == other.screening_gate && extra == other.extra &&
         uncertainty == other.uncertainty && hash == other.hash &&
         time == other.time && state == other.state &&
         unit_name == other.unit_name && characteristic == other.characteristic;
}

bool DeviceCharacteristic::operator!=(const DeviceCharacteristic &other) const {
  return !(*this == other);
}

} // namespace falcon::database
