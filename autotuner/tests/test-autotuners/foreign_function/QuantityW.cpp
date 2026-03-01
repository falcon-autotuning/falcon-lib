#include "falcon-autotuner/RuntimeValue.hpp"
#include <memory>

using namespace falcon::autotuner;

struct Quantity {
  int64_t a;
  int64_t b;
};

extern "C" {

// Constructor: STRUCTQuantityNew
// Called as:   Quantity q = Quantity.New(a);
// Params:      params["a"] = int64_t
// Returns:     FunctionResult with one element: shared_ptr<StructInstance>
//              whose native_handle owns the Quantity.
FunctionResult STRUCTQuantityNew(ParameterMap params) {
  int64_t a = std::get<int64_t>(params.at("a"));
  // b is optional; default to 0 if not provided
  int64_t b = 0;
  if (params.count("b"))
    b = std::get<int64_t>(params.at("b"));

  auto q = std::make_shared<Quantity>(a, b);
  // Wrap in a StructInstance that owns q via native_handle.
  // Fields map is left empty — all data lives in the native object.
  auto inst = StructInstance::from_native("Quantity", q);
  return FunctionResult{RuntimeValue{inst}};
}

// NewWithB: takes both a and b
FunctionResult STRUCTQuantityNewWithB(ParameterMap params) {
  int64_t a = std::get<int64_t>(params.at("a"));
  int64_t b = std::get<int64_t>(params.at("b"));
  auto q = std::make_shared<Quantity>(a, b);
  return FunctionResult{
      RuntimeValue{StructInstance::from_native("Quantity", q)}};
}

// Instance method: STRUCTQuantityValue
// Called as:   int v = q.Value();
// Params:      params["this"] = shared_ptr<StructInstance> with native_handle
FunctionResult STRUCTQuantityValue(ParameterMap params) {
  auto inst = std::get<std::shared_ptr<StructInstance>>(params.at("this"));
  auto q = inst->get_native<Quantity>();
  return FunctionResult{RuntimeValue{q->a}};
}

} // extern "C"
