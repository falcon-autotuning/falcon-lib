#pragma once

#include "MeasurementRoutine.hpp"
#include "ParameterMap.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace falcon {
namespace autotuner {

/**
 * @brief Unique identifier for a state
 */
struct StateId {
  std::string autotuner_name;
  std::string state_name;

  StateId() = default;
  StateId(std::string at, std::string st)
      : autotuner_name(std::move(at)), state_name(std::move(st)) {}

  std::string full_name() const { return autotuner_name + "::" + state_name; }

  bool operator==(const StateId &other) const {
    return autotuner_name == other.autotuner_name &&
           state_name == other.state_name;
  }

  bool operator<(const StateId &other) const {
    if (autotuner_name != other.autotuner_name) {
      return autotuner_name < other.autotuner_name;
    }
    return state_name < other.state_name;
  }
};

/**
 * @brief Transition condition
 */
class TransitionCondition {
public:
  using Func = std::function<bool(const ParameterMap &)>;

  TransitionCondition(Func func, std::string description = "")
      : func_(std::move(func)), description_(std::move(description)) {}

  bool evaluate(const ParameterMap &params) const { return func_(params); }

  std::string description() const { return description_; }

  // Factory methods for common conditions
  static TransitionCondition always() {
    return TransitionCondition([](const ParameterMap &) { return true; },
                               "always");
  }

  static TransitionCondition never() {
    return TransitionCondition([](const ParameterMap &) { return false; },
                               "never");
  }

  template <typename T>
  static TransitionCondition equals(const std::string &param, T value) {
    return TransitionCondition(
        [param, value](const ParameterMap &params) {
          auto val = params.try_get<T>(param);
          return val && *val == value;
        },
        param + " == " + std::to_string(value));
  }

  template <typename T>
  static TransitionCondition greater_than(const std::string &param, T value) {
    return TransitionCondition(
        [param, value](const ParameterMap &params) {
          auto val = params.try_get<T>(param);
          return val && *val > value;
        },
        param + " > " + std::to_string(value));
  }

  template <typename T>
  static TransitionCondition less_than(const std::string &param, T value) {
    return TransitionCondition(
        [param, value](const ParameterMap &params) {
          auto val = params.try_get<T>(param);
          return val && *val < value;
        },
        param + " < " + std::to_string(value));
  }

  static TransitionCondition param_exists(const std::string &param) {
    return TransitionCondition(
        [param](const ParameterMap &params) { return params.has(param); },
        "has " + param);
  }

private:
  Func func_;
  std::string description_;
};
// String specialization for equals
template <>
inline TransitionCondition
TransitionCondition::equals<std::string>(const std::string &param,
                                         std::string value) {
  return TransitionCondition(
      [param, value](const ParameterMap &params) {
        auto v = params.try_get<std::string>(param);
        return v && *v == value;
      },
      param + " == \"" + value + "\"");
}
/**
 * @brief State transition
 */
struct StateTransition {
  StateId target;
  TransitionCondition condition;
  int priority = 0; // Higher priority evaluated first

  StateTransition(StateId tgt, TransitionCondition cond, int prio = 0)
      : target(std::move(tgt)), condition(std::move(cond)), priority(prio) {}
};

/**
 * @brief A state in the autotuner state machine
 */
class StateNode {
public:
  explicit StateNode(StateId id) : id_(std::move(id)) {}

  const StateId &id() const { return id_; }

  void set_measurement(std::shared_ptr<MeasurementRoutine> measurement) {
    measurement_ = std::move(measurement);
  }

  std::shared_ptr<MeasurementRoutine> measurement() const {
    return measurement_;
  }

  void add_transition(StateTransition transition) {
    transitions_.push_back(std::move(transition));
    // Sort by priority (descending)
    std::sort(transitions_.begin(), transitions_.end(),
              [](const StateTransition &a, const StateTransition &b) {
                return a.priority > b.priority;
              });
  }

  void set_default_transition(StateId target) {
    add_transition(
        StateTransition(std::move(target), TransitionCondition::always(), 0));
  }

  const std::vector<StateTransition> &transitions() const {
    return transitions_;
  }

  std::optional<StateId>
  evaluate_transitions(const ParameterMap &params) const {
    for (const auto &transition : transitions_) {
      if (transition.condition.evaluate(params)) {
        return transition.target;
      }
    }
    return std::nullopt;
  }

  bool is_terminal() const { return transitions_.empty(); }

private:
  StateId id_;
  std::shared_ptr<MeasurementRoutine> measurement_;
  std::vector<StateTransition> transitions_;
};

} // namespace autotuner
} // namespace falcon
