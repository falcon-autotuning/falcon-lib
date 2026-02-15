#pragma once

#include "ParameterMap.hpp"
#include "StateNode.hpp"
#include <map>
#include <memory>
#include <string>

namespace falcon {
namespace autotuner {

/**
 * @brief Result of autotuner execution
 */
struct AutotunerResult {
  bool success = false;
  std::string error_message;
  StateId final_state;
  ParameterMap final_params;
  int transition_count = 0;
};

/**
 * @brief Main autotuner class
 */
class Autotuner {
public:
  explicit Autotuner(std::string name) : name_(std::move(name)) {}

  std::string name() const { return name_; }

  /**
   * @brief Create a new state
   */
  std::shared_ptr<StateNode> create_state(const std::string &state_name) {
    StateId id(name_, state_name);
    auto state = std::make_shared<StateNode>(id);
    states_[state_name] = state;
    return state;
  }

  /**
   * @brief Get a state by name
   */
  std::shared_ptr<StateNode> get_state(const std::string &state_name) const {
    auto it = states_.find(state_name);
    return (it != states_.end()) ? it->second : nullptr;
  }

  /**
   * @brief Get all states
   */
  std::map<std::string, std::shared_ptr<StateNode>> get_all_states() const {
    return states_;
  }

  /**
   * @brief Set entry state
   */
  void set_entry_state(const std::string &state_name) {
    entry_state_ = StateId(name_, state_name);
  }

  StateId entry_state() const { return entry_state_; }

  /**
   * @brief Helper to create StateId for this autotuner
   */
  StateId state_id(const std::string &state_name) const {
    return StateId(name_, state_name);
  }

  /**
   * @brief Run the autotuner
   */
  AutotunerResult run(ParameterMap initial_params = ParameterMap(),
                      int max_transitions = 1000);

private:
  std::string name_;
  std::map<std::string, std::shared_ptr<StateNode>> states_;
  StateId entry_state_;
};

/**
 * @brief Registry for autotuners (enables cross-autotuner transitions)
 */
class AutotunerRegistry {
public:
  static AutotunerRegistry &instance() {
    static AutotunerRegistry registry;
    return registry;
  }

  void register_autotuner(std::shared_ptr<Autotuner> autotuner) {
    autotuners_[autotuner->name()] = autotuner;
  }

  std::shared_ptr<Autotuner> get_autotuner(const std::string &name) const {
    auto it = autotuners_.find(name);
    return (it != autotuners_.end()) ? it->second : nullptr;
  }

  std::shared_ptr<StateNode> get_state(const StateId &id) const {
    auto autotuner = get_autotuner(id.autotuner_name);
    return autotuner ? autotuner->get_state(id.state_name) : nullptr;
  }

  void clear() { autotuners_.clear(); }

private:
  AutotunerRegistry() = default;
  std::map<std::string, std::shared_ptr<Autotuner>> autotuners_;
};

} // namespace autotuner
} // namespace falcon
