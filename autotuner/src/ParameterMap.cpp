#include "falcon-autotuner/ParameterMap.hpp"

namespace falcon::autotuner {

void ParameterMap::set(const std::string &key, const int64_t &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key, const double &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key, const bool &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key, const std::string &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key, const ConnectionSP &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key, const ConnectionsSP &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key,
                       const database::DeviceCharacteristic &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key,
                       const database::DeviceCharacteristicQuery &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key,
                       const falcon_core::math::QuantitySP &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(
    const std::string &key,
    const falcon_core::physics::config::core::ConfigSP &value,
    const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}
void ParameterMap::set(const std::string &key, const Value &value,
                       const atc::ParamType &type) {
  params_[key] = value;
  types_[key] = type;
}

bool ParameterMap::has(const std::string &key) const {
  return params_.find(key) != params_.end();
}

std::vector<std::string> ParameterMap::keys() const {
  std::vector<std::string> result;
  result.reserve(params_.size());
  for (const auto &kv : params_) {
    result.push_back(kv.first);
  }
  return result;
}

void ParameterMap::merge(const ParameterMap &other) {
  for (const auto &kv : other.params_) {
    params_[kv.first] = kv.second;
    types_[kv.first] = other.types_.at(kv.first);
  }
}

void ParameterMap::remove(const std::string &key) {
  params_.erase(key);
  types_.erase(key);
}

void ParameterMap::clear() {
  params_.clear();
  types_.clear();
}

size_t ParameterMap::size() const { return params_.size(); }

nlohmann::json ParameterMap::to_json() const {
  nlohmann::json result = nlohmann::json::object();
  for (const auto &key : keys()) {
    const auto &value = params_.at(key);
    if (auto v = std::get_if<int64_t>(&value)) {
      result[key] = *v;
    } else if (auto v = std::get_if<double>(&value)) {
      result[key] = *v;
    } else if (auto v = std::get_if<bool>(&value)) {
      result[key] = *v;
    } else if (auto v = std::get_if<std::string>(&value)) {
      result[key] = *v;
    } else if (auto v = std::get_if<database::DeviceCharacteristic>(&value)) {
      result[key] = v->to_json();
    } else {
      // falcon-core objects cannot be serialized to JSON easily
      result[key] = nullptr;
    }
  }
  return result;
}

ParameterMap ParameterMap::from_json(const nlohmann::json &j) {
  ParameterMap map;
  for (auto it = j.begin(); it != j.end(); ++it) {
    const auto &key = it.key();
    const auto &value = it.value();
    if (value.is_number_integer()) {
      map.set(key, value.get<int64_t>(), atc::ParamType::Int);
    } else if (value.is_number_float()) {
      map.set(key, value.get<double>(), atc::ParamType::Float);
    } else if (value.is_boolean()) {
      map.set(key, value.get<bool>(), atc::ParamType::Bool);
    } else if (value.is_string()) {
      map.set(key, value.get<std::string>(), atc::ParamType::String);
    }
    // Skip unsupported types
  }
  return map;
}

atc::ParamType ParameterMap::get_type(const std::string &key) const {
  auto it = types_.find(key);
  if (it == types_.end()) {
    throw std::runtime_error("Type not found for parameter: " + key);
  }
  return it->second;
}

} // namespace falcon::autotuner
