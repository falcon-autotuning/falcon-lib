#pragma once
#include "falcon-atc/AST.hpp" // for ParamType
#include "falcon-database/DeviceCharacteristic.hpp"
#include "falcon_core/math/Quantity.hpp"
#include "falcon_core/physics/config/core/Config.hpp"
#include <map>
#include <memory>
#include <nlohmann/json.hpp>
#include <optional>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unordered_map>
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
                             ConnectionsSP, database::DeviceCharacteristic,
                             database::DeviceCharacteristicQuery,
                             falcon_core::math::QuantitySP,
                             falcon_core::physics::config::core::ConfigSP>;
  static atc::ParamType deduce_type(const Value &v) {
    return std::visit(
        [](const auto &val) -> atc::ParamType {
          using T = std::decay_t<decltype(val)>;
          if constexpr (std::is_same_v<T, int64_t>)
            return atc::ParamType::Int;
          if constexpr (std::is_same_v<T, double>)
            return atc::ParamType::Float;
          if constexpr (std::is_same_v<T, bool>)
            return atc::ParamType::Bool;
          if constexpr (std::is_same_v<T, std::string>)
            return atc::ParamType::String;
          if constexpr (std::is_same_v<T, ConnectionSP>)
            return atc::ParamType::Connection;
          if constexpr (std::is_same_v<T, ConnectionsSP>)
            return atc::ParamType::Connections;
          if constexpr (std::is_same_v<T, database::DeviceCharacteristic>)
            return atc::ParamType::Quantity;
          return atc::ParamType::Int; // fallback, should not happen
        },
        v);
  }

  /**
   * @brief Set a parameter value and its type
   */
  void set(const std::string &key, const int64_t &value,
           const atc::ParamType &type = atc::ParamType::Int);
  void set(const std::string &key, const double &value,
           const atc::ParamType &type = atc::ParamType::Float);
  void set(const std::string &key, const bool &value,
           const atc::ParamType &type = atc::ParamType::Bool);
  void set(const std::string &key, const std::string &value,
           const atc::ParamType &type = atc::ParamType::String);
  void set(const std::string &key, const ConnectionSP &value,
           const atc::ParamType &type = atc::ParamType::Connection);
  void set(const std::string &key, const ConnectionsSP &value,
           const atc::ParamType &type = atc::ParamType::Connections);
  void set(const std::string &key, const database::DeviceCharacteristic &value,
           const atc::ParamType &type = atc::ParamType::DeviceCharacteristic);
  void
  set(const std::string &key, const database::DeviceCharacteristicQuery &value,
      const atc::ParamType &type = atc::ParamType::DeviceCharacteristicQuery);
  void set(const std::string &key, const falcon_core::math::QuantitySP &value,
           const atc::ParamType &type = atc::ParamType::Quantity);
  void set(const std::string &key,
           const falcon_core::physics::config::core::ConfigSP &value,
           const atc::ParamType &type = atc::ParamType::Config);
  void set(const std::string &key, const Value &value,
           const atc::ParamType &type);

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

  /**
   * @brief Get the type of a parameter (throws if not found)
   */
  atc::ParamType get_type(const std::string &key) const;

private:
  std::map<std::string, Value> params_;
  std::unordered_map<std::string, atc::ParamType> types_;
};
} // namespace falcon::autotuner
