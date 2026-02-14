#include "falcon-autotuner/ParameterMap.hpp"

namespace falcon::autotuner {

json ParameterMap::to_json() const {
  json result = json::object();

  for (const auto &key : keys()) {
    const auto &value = data_.at(key);

    // Try to convert common types to JSON
    if (auto *v = std::any_cast<int>(&value)) {
      result[key] = *v;
    } else if (auto *v = std::any_cast<int64_t>(&value)) {
      result[key] = *v;
    } else if (auto *v = std::any_cast<double>(&value)) {
      result[key] = *v;
    } else if (auto *v = std::any_cast<float>(&value)) {
      result[key] = *v;
    } else if (auto *v = std::any_cast<bool>(&value)) {
      result[key] = *v;
    } else if (auto *v = std::any_cast<std::string>(&value)) {
      result[key] = *v;
    } else {
      // Unsupported type - store as null with type info
      result[key] = json::object(
          {{"_type", "unsupported"}, {"_type_name", types_.at(key).name()}});
    }
  }

  return result;
}

ParameterMap ParameterMap::from_json(const json &j) {
  ParameterMap map;

  for (auto it = j.begin(); it != j.end(); ++it) {
    const auto &key = it.key();
    const auto &value = it.value();

    if (value.is_number_integer()) {
      map.set(key, value.get<int64_t>());
    } else if (value.is_number_float()) {
      map.set(key, value.get<double>());
    } else if (value.is_boolean()) {
      map.set(key, value.get<bool>());
    } else if (value.is_string()) {
      map.set(key, value.get<std::string>());
    }
    // Skip unsupported types
  }

  return map;
}

} // namespace falcon::autotuner
