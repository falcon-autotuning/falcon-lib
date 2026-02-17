#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace falcon::autotuner {

/**
 * @brief Represents a device characteristic from the spec database
 */
struct DeviceCharacteristic {
  std::string name;
  std::vector<std::string> indexes;
  double uncertainty = 0.0;
  std::string hash;
  int64_t record_time = 0;
  std::string device_state;
  nlohmann::json characteristic;
  std::string unit_name;

  nlohmann::json to_json() const {
    return {{"name", name},
            {"indexes", indexes},
            {"uncertainty", uncertainty},
            {"hash", hash},
            {"recordtime", record_time},
            {"device_state", device_state},
            {"device_characteristic", characteristic},
            {"unit_name", unit_name}};
  }

  static DeviceCharacteristic from_json(const nlohmann::json &j) {
    DeviceCharacteristic dc;
    dc.name = j.value("name", "");
    dc.indexes = j.value("indexes", std::vector<std::string>{});
    dc.uncertainty = j.value("uncertainty", 0.0);
    dc.hash = j.value("hash", "");
    dc.record_time = j.value("recordtime", 0l);
    dc.device_state = j.value("device_state", "");
    dc.characteristic =
        j.value("device_characteristic", nlohmann::json::object());
    dc.unit_name = j.value("unit_name", "");
    return dc;
  }

  bool operator==(const DeviceCharacteristic &other) const {
    return name == other.name && indexes == other.indexes &&
           uncertainty == other.uncertainty && hash == other.hash &&
           record_time == other.record_time &&
           device_state == other.device_state &&
           characteristic == other.characteristic &&
           unit_name == other.unit_name;
  }
};

} // namespace falcon::autotuner
