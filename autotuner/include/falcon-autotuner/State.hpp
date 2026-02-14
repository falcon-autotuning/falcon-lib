#pragma once

#include "falcon-autotuner/ParameterMap.hpp"
#include <string>
#include <utility>

namespace falcon::autotuner {

/**
 * @brief Represents a state in the autotuner state machine
 */
class State {
public:
  explicit State(std::string name) : name_(std::move(name)) {}

  [[nodiscard]] const std::string &name() const { return name_; }

  ParameterMap &local_params() { return local_params_; }
  [[nodiscard]] const ParameterMap &local_params() const {
    return local_params_;
  }

  void set_parent_params(const ParameterMap &parent) {
    parent_params_ = parent.create_readonly_view();
  }

  [[nodiscard]] const ParameterMap &parent_params() const {
    return parent_params_;
  }

  /**
   * @brief Get combined view of local and parent parameters
   * Local parameters shadow parent parameters with same name
   */
  [[nodiscard]] ParameterMap get_combined_params() const {
    ParameterMap combined = parent_params_;
    combined.merge(local_params_);
    return combined;
  }

private:
  std::string name_;
  ParameterMap local_params_;
  ParameterMap parent_params_;
};

} // namespace falcon::autotuner
