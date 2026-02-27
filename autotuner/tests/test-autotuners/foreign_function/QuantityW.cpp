#include "falcon-autotuner/RuntimeValue.hpp"
struct Quantity {
  int64_t a;
  int64_t b;
};

falcon::autotuner::FunctionResult
STRUCTQuantityNew(falcon::autotuner::ParameterMap params) {
  int64_t a = std::get<int64_t>(params.at("a"));
  int64_t b = std::get<int64_t>(params.at("b"));
  std::shared_ptr<Quantity> q = std::make_shared<Quantity>(a, b);
  return falcon::autotuner::FunctionResult{q};
}

falcon::autotuner::FunctionResult
STRUCTQuantityValue(falcon::autotuner::ParameterMap params) {
  using QuantitySP = std::shared_ptr<Quantity>;
  QuantitySP q = std::get<QuantitySP>(params.at("this"));
  return falcon::autotuner::FunctionResult{q->a};
}
