#pragma once

#include "falcon-autotuner/MeasurementRoutine.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace falcon::autotuner {

class Autotuner;

/**
 * @brief Represents a state identifier (autotuner_name::state_name)
 */
struct StateId {
  std::string autotuner;
  std::string state;

  StateId(std::string a, std::string s)
      : autotuner(std::move(a)), state(std::move(s)) {}
};

/**
 * @brief Condition for a state transition
 */
struct TransitionCondition {
  using Func = std::function<bool(const ParameterMap &)>;
  Func func;
  std::string description;

  TransitionCondition(Func f, std::string desc)
      : func(std::move(f)), description(std::move(desc)) {}
};

/**
 * @brief Represents a transition between states
 */
struct StateTransition {
  StateId target;
  TransitionCondition condition;

  StateTransition(StateId tgt, TransitionCondition cond)
      : target(std::move(tgt)), condition(std::move(cond)) {}
};

/**
 * @brief A single state in the autotuner state machine
 */
class State {
public:
  explicit State(std::string name);

  void set_measurement(std::shared_ptr<MeasurementRoutine> measurement);
  void add_transition(StateTransition transition);
  void set_terminal(bool terminal);

  const std::string &name() const { return name_; }
  bool is_terminal() const { return is_terminal_; }
  const std::vector<StateTransition> &transitions() const {
    return transitions_;
  }
  std::shared_ptr<MeasurementRoutine> measurement() const {
    return measurement_;
  }

private:
  std::string name_;
  std::shared_ptr<MeasurementRoutine> measurement_;
  std::vector<StateTransition> transitions_;
  bool is_terminal_ = false;
};

/**
 * @brief Main Autotuner class
 */
class Autotuner {
public:
  explicit Autotuner(std::string name);

  std::shared_ptr<State> create_state(const std::string &name);
  void set_entry_state(const std::string &name);

  const std::string &name() const { return name_; }
  std::shared_ptr<State> get_state(const std::string &name) const;
  const std::string &entry_state() const { return entry_state_; }

private:
  std::string name_;
  std::string entry_state_;
  std::map<std::string, std::shared_ptr<State>> states_;
};

} // namespace falcon::autotuner
