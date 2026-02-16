#include "falcon-autotuner/Autotuner.hpp"
#include <stdexcept>

namespace falcon::autotuner {

// State Implementation

State::State(std::string name) : name_(std::move(name)) {}

void State::set_measurement(std::shared_ptr<MeasurementRoutine> measurement) {
  measurement_ = std::move(measurement);
}

void State::add_transition(StateTransition transition) {
  transitions_.push_back(std::move(transition));
}

void State::set_terminal(bool terminal) { is_terminal_ = terminal; }

// Autotuner Implementation

Autotuner::Autotuner(std::string name) : name_(std::move(name)) {}

std::shared_ptr<State> Autotuner::create_state(const std::string &name) {
  auto state = std::make_shared<State>(name);
  states_[name] = state;
  return state;
}

void Autotuner::set_entry_state(const std::string &name) {
  if (states_.find(name) == states_.end()) {
    throw std::runtime_error("State not found: " + name);
  }
  entry_state_ = name;
}

std::shared_ptr<State> Autotuner::get_state(const std::string &name) const {
  auto it = states_.find(name);
  if (it == states_.end()) {
    return nullptr;
  }
  return it->second;
}

} // namespace falcon::autotuner
