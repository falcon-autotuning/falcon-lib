#include "falcon-autotuner/FfiHelpers.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::ffi::wrapper;

extern "C" {

void Adder(const FalconParamEntry *params, int32_t param_count,
           FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  int64_t a = std::get<int64_t>(pm.at("a"));
  int64_t b = std::get<int64_t>(pm.at("b"));
  pack_results(FunctionResult{a + b}, out, 16, out_count);
}

void Multiplier(const FalconParamEntry *params, int32_t param_count,
                FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  int64_t a = std::get<int64_t>(pm.at("a"));
  int64_t b = std::get<int64_t>(pm.at("b"));
  pack_results(FunctionResult{a * b}, out, 16, out_count);
}

} // extern "C"
