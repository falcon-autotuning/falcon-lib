#include "falcon-autotuner/ParameterMap.hpp"
#include <stdexcept>

namespace falcon {
namespace autotuner {

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
  for (const auto &[key, _] : params_) {
    result.push_back(key);
  }
  return result;
}

void ParameterMap::merge(const ParameterMap &other) {
  for (const auto &[key, value] : other.params_) {
    params_[key] = value;
  }
}

void ParameterMap::remove(const std::string &key) { params_.erase(key); }

void ParameterMap::clear() { params_.clear(); }

size_t ParameterMap::size() const { return params_.size(); }

nlohmann::json ParameterMap::to_json() const {
  nlohmann::json j = nlohmann::json::object();

  for (const auto &[key, value] : params_) {
    std::visit([&](auto &&val) { j[key] = val; }, value);
  }

  return j;
}

ParameterMap ParameterMap::from_json(const nlohmann::json &j) {
  ParameterMap result;

  for (auto it = j.begin(); it != j.end(); ++it) {
    const auto &key = it.key();
    const auto &value = it.value();

    if (value.is_number_integer()) {
      result.set(key, value.get<int64_t>());
    } else if (value.is_number_float()) {
      result.set(key, value.get<double>());
    } else if (value.is_boolean()) {
      result.set(key, value.get<bool>());
    } else if (value.is_string()) {
      result.set(key, value.get<std::string>());
    }
  }

  return result;
}

} // namespace autotuner
} // namespace falcon
