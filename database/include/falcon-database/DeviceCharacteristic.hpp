#pragma once

#include <cstdint>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace falcon::database {

using json = nlohmann::json;

/**
 * @brief Represents a possible query for a device characteristic which is
 * stored in the globals database
 */
struct DeviceCharacteristicQuery {
  std::optional<std::string> scope;
  std::optional<std::string> name;
  std::optional<std::string> barrier_gate;
  std::optional<std::string> plunger_gate;
  std::optional<std::string> reservoir_gate;
  std::optional<std::string> screening_gate;
  std::optional<std::string> extra;
  std::optional<double> uncertainty;
  std::optional<std::string> hash;
  std::optional<int64_t> time;
  std::optional<std::string> state;
  std::optional<std::string> unit_name;
  // No characteristic field here
  bool operator==(const DeviceCharacteristicQuery &other) const;
  bool operator!=(const DeviceCharacteristicQuery &other) const;
};
/**
 * @brief Represents a device characteristic which is stored in the globals
 * database
 */
struct DeviceCharacteristic {
  std::string scope;
  std::string name;
  std::optional<std::string> barrier_gate;
  std::optional<std::string> plunger_gate;
  std::optional<std::string> reservoir_gate;
  std::optional<std::string> screening_gate;
  std::optional<std::string> extra;
  std::optional<double> uncertainty;
  std::optional<std::string> hash;
  std::optional<int64_t> time;
  std::optional<std::string> state;
  std::optional<std::string> unit_name;
  json characteristic;

  [[nodiscard]] json to_json() const;
  static DeviceCharacteristic from_json(const json &json);
  bool operator==(const DeviceCharacteristic &other) const;
  bool operator!=(const DeviceCharacteristic &other) const;
};

} // namespace falcon::database
