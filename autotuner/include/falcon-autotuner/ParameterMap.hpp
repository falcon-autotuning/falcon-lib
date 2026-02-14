#pragma once

#include <any>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <typeindex>

namespace falcon::autotuner {

using json = nlohmann::json;

/**
 * @brief Type-erased parameter storage with type safety
 *
 * Allows storing heterogeneous types in a map with runtime type checking
 */
class ParameterMap {
public:
  ParameterMap() = default;

  /**
   * @brief Set a parameter with type information
   */
  template <typename T> void set(const std::string &key, T &&value) {
    data_[key] = std::make_any<std::decay_t<T>>(std::forward<T>(value));
    types_[key] = std::type_index(typeid(std::decay_t<T>));
  }

  /**
   * @brief Get a parameter with type checking
   * @throws std::bad_any_cast if type mismatch
   */
  template <typename T> T get(const std::string &key) const {
    auto it = data_.find(key);
    if (it == data_.end()) {
      throw std::runtime_error("Parameter not found: " + key);
    }
    return std::any_cast<T>(it->second);
  }

  /**
   * @brief Try to get a parameter, returns nullopt if not found or type
   * mismatch
   */
  template <typename T> std::optional<T> try_get(const std::string &key) const {
    auto it = data_.find(key);
    if (it == data_.end()) {
      return std::nullopt;
    }
    try {
      return std::any_cast<T>(it->second);
    } catch (const std::bad_any_cast &) {
      return std::nullopt;
    }
  }

  /**
   * @brief Check if a parameter exists
   */
  [[nodiscard]] bool has(const std::string &key) const {
    return data_.find(key) != data_.end();
  }

  /**
   * @brief Get the type of a parameter
   */
  [[nodiscard]] std::optional<std::type_index>
  get_type(const std::string &key) const {
    auto it = types_.find(key);
    if (it == types_.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  /**
   * @brief Remove a parameter
   */
  void remove(const std::string &key) {
    data_.erase(key);
    types_.erase(key);
  }

  /**
   * @brief Get all parameter keys
   */
  [[nodiscard]] std::vector<std::string> keys() const {
    std::vector<std::string> result;
    result.reserve(data_.size());
    for (const auto &[key, _] : data_) {
      result.push_back(key);
    }
    return result;
  }

  /**
   * @brief Clear all parameters
   */
  void clear() {
    data_.clear();
    types_.clear();
  }

  /**
   * @brief Merge another parameter map (this one takes precedence)
   */
  void merge(const ParameterMap &other) {
    for (const auto &key : other.keys()) {
      if (!has(key)) {
        data_[key] = other.data_.at(key);
        types_[key] = other.types_.at(key);
      }
    }
  }

  /**
   * @brief Create a read-only view (copies all parameters)
   */
  [[nodiscard]] ParameterMap create_readonly_view() const {
    ParameterMap view;
    view.data_ = data_;
    view.types_ = types_;
    view.readonly_ = true;
    return view;
  }

  /**
   * @brief Check if this is a read-only view
   */
  [[nodiscard]] bool is_readonly() const { return readonly_; }

  /**
   * @brief Convert to JSON (for serialization)
   * Note: Only supports JSON-serializable types
   */
  [[nodiscard]] json to_json() const;

  /**
   * @brief Load from JSON
   */
  static ParameterMap from_json(const json &j);

private:
  std::map<std::string, std::any> data_;
  std::map<std::string, std::type_index> types_;
  bool readonly_ = false;

  void check_readonly() const {
    if (readonly_) {
      throw std::runtime_error("Cannot modify read-only parameter map");
    }
  }
};

} // namespace falcon::autotuner
