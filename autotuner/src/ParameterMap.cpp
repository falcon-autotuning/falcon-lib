#include "falcon-autotuner/ParameterMap.hpp"

namespace falcon::autotuner {

void ParameterMap::set(const std::string &key, int64_t value) {
  params_[key] = value;
}

void ParameterMap::set(const std::string &key, double value) {
  params_[key] = value;
}

void ParameterMap::set(const std::string &key, bool value) {
  params_[key] = value;
}

void ParameterMap::set(const std::string &key, std::string value) {
  params_[key] = std::move(value);
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
  }
}

void ParameterMap::remove(const std::string &key) { params_.erase(key); }

void ParameterMap::clear() { params_.clear(); }

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
    } else {
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
