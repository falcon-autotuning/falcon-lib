#include "falcon-autotuner/RuntimeValue.hpp"

extern "C" {

falcon::autotuner::FunctionResult
Adder(falcon::autotuner::ParameterMap params) {
  int64_t a = std::get<int64_t>(params.at("a"));
  int64_t b = std::get<int64_t>(params.at("b"));
  return falcon::autotuner::FunctionResult{a + b};
}

falcon::autotuner::FunctionResult
Multiplier(falcon::autotuner::ParameterMap params) {
  int64_t a = std::get<int64_t>(params.at("a"));
  int64_t b = std::get<int64_t>(params.at("b"));
  return falcon::autotuner::FunctionResult{a * b};
}

} // extern "C"
