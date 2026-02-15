#pragma once

#include "falcon-autotuner/ParameterMap.hpp"
#include <functional>
#include <memory>
#include <string>

namespace falcon::autotuner {

class StateMachine {
public:
  virtual ~StateMachine() = default;
  virtual void step() = 0;
  virtual bool is_terminal() const = 0;
  virtual std::string current_state() const = 0;
  virtual ParameterMap &get_params() = 0;
};

using AutotunerBuilder =
    std::function<std::unique_ptr<StateMachine>(ParameterMap)>;

class RuntimeEngine {
public:
  void load_library(const std::string &path);
  void load_snapshot(const std::string &snapshot_file);
  void run(const std::string &initial_autotuner);

private:
  void *library_handle_ = nullptr;
  ParameterMap snapshot_params_;
};

} // namespace falcon::autotuner
