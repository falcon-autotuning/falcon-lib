#pragma once

#include "falcon-database/DeviceCharacteristic.hpp"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

namespace falcon_core::physics::device_structures {
class Connection;
class Connections;
} // namespace falcon_core::physics::device_structures

namespace falcon::autotuner {

using ConnectionSP =
    std::shared_ptr<::falcon_core::physics::device_structures::Connection>;
using ConnectionsSP =
    std::shared_ptr<::falcon_core::physics::device_structures::Connections>;

/**
 * @brief Type-safe parameter storage for autotuner state
 */
class ParameterMap {
public:
  using Value = std::variant<int64_t, double, bool, std::string, ConnectionSP,
                             ConnectionsSP, database::DeviceCharacteristic>;

  static std::string value_to_string(const Value &v) {
    return std::visit(
        [](const auto &val) -> std::string {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, std::string>) {
            return val;
          } else if constexpr (std::is_same_v<T, bool>) {
            return val ? "true" : "false";
          } else if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(val);
          } else {
            return "<unprintable>";
          }
        },
        v);
  }
  /**
   * @brief Set a parameter value
   */
  void set(const std::string &key, int64_t value);
  void set(const std::string &key, double value);
  void set(const std::string &key, bool value);
  void set(const std::string &key, std::string value);
  void set(const std::string &key, ConnectionSP value);
  void set(const std::string &key, ConnectionsSP value);
  void set(const std::string &key, database::DeviceCharacteristic value);
  void set(const std::string &key, Value value);

  /**
   * @brief Get a parameter value (throws if not found or wrong type)
   */
  template <typename T> T get(const std::string &key) const {
    auto it = params_.find(key);
    if (it == params_.end()) {
      throw std::runtime_error("Parameter not found: " + key);
    }

    if constexpr (std::is_same_v<T, Value>) {
      return it->second;
    } else {
      try {
        return std::get<T>(it->second);
      } catch (const std::bad_variant_access &) {
        throw std::runtime_error("Type mismatch for parameter: " + key);
      }
    }
  }

  /**
   * @brief Try to get a parameter value (returns nullopt if not found or wrong
   * type)
   */
  template <typename T> std::optional<T> try_get(const std::string &key) const {
    auto it = params_.find(key);
    if (it == params_.end()) {
      return std::nullopt;
    }

    try {
      return std::get<T>(it->second);
    } catch (const std::bad_variant_access &) {
      return std::nullopt;
    }
  }

  /**
   * @brief Check if parameter exists
   */
  [[nodiscard]] bool has(const std::string &key) const;

  /**
   * @brief Get all parameter keys
   */
  [[nodiscard]] std::vector<std::string> keys() const;

  /**
   * @brief Merge another ParameterMap into this one (overwrites existing)
   */
  void merge(const ParameterMap &other);

  /**
   * @brief Remove a parameter
   */
  void remove(const std::string &key);

  /**
   * @brief Clear all parameters
   */
  void clear();

  /**
   * @brief Get number of parameters
   */
  [[nodiscard]] size_t size() const;

  /**
   * @brief Convert to JSON
   */
  [[nodiscard]] nlohmann::json to_json() const;

  /**
   * @brief Create from JSON
   */
  static ParameterMap from_json(const nlohmann::json &j);

private:
  std::map<std::string, Value> params_;
};

} // namespace falcon::autotuner
