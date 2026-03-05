#include "falcon_core/generic/FArray.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using FArray   = falcon_core::generic::FArray;
using FArraySP = std::shared_ptr<FArray>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_farray(FArraySP arr, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "FArray";
  out[0].value.opaque.ptr       = new FArraySP(std::move(arr));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<FArraySP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: FArray, other: FArray) -> (bool equal)
void STRUCTFArrayEqual(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<FArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: FArray, other: FArray) -> (bool notequal)
void STRUCTFArrayNotEqual(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<FArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── Arithmetic: Times ─────────────────────────────────────────────────────────

// Times(this: FArray, factor: float) -> (FArray scaled_state)
void STRUCTFArrayTimesFloat(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm    = unpack_params(params, param_count);
  auto self  = get_opaque<FArray>(params, param_count, "this");
  double fac = std::get<double>(pm.at("factor"));
  pack_farray(std::make_shared<FArray>(*self * fac), out, oc);
}

// Times(this: FArray, factor: int) -> (FArray scaled_state)
void STRUCTFArrayTimesInt(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto pm     = unpack_params(params, param_count);
  auto self   = get_opaque<FArray>(params, param_count, "this");
  int64_t fac = std::get<int64_t>(pm.at("factor"));
  pack_farray(std::make_shared<FArray>(*self * static_cast<int>(fac)), out, oc);
}

// Times(this: FArray, factor: Quantity) -> (FArray scaled_state)
void STRUCTFArrayTimesQuantity(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self  = get_opaque<FArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "factor");
  pack_farray(std::make_shared<FArray>(
      *self * std::static_pointer_cast<falcon_core::math::Quantity>(other)),
              out, oc);
}

// ── Arithmetic: Divides ───────────────────────────────────────────────────────

// Divides(this: FArray, divisor: float) -> (FArray scaled_state)
void STRUCTFArrayDividesFloat(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto pm    = unpack_params(params, param_count);
  auto self  = get_opaque<FArray>(params, param_count, "this");
  double div = std::get<double>(pm.at("divisor"));
  pack_farray(std::make_shared<FArray>(*self / div), out, oc);
}

// Divides(this: FArray, divisor: int) -> (FArray scaled_state)
void STRUCTFArrayDividesInt(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm     = unpack_params(params, param_count);
  auto self   = get_opaque<FArray>(params, param_count, "this");
  int64_t div = std::get<int64_t>(pm.at("divisor"));
  pack_farray(std::make_shared<FArray>(*self / static_cast<int>(div)), out, oc);
}

// Divides(this: FArray, divisor: Quantity) -> (FArray scaled_state)
void STRUCTFArrayDividesQuantity(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self  = get_opaque<FArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "divisor");
  pack_farray(std::make_shared<FArray>(
      *self / std::static_pointer_cast<falcon_core::math::Quantity>(other)),
              out, oc);
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: FArray, exponent: float) -> (FArray powered_state)
void STRUCTFArrayPower(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto pm     = unpack_params(params, param_count);
  auto self   = get_opaque<FArray>(params, param_count, "this");
  int64_t exp = std::get<int64_t>(pm.at("exponent"));
  pack_farray(std::make_shared<FArray>(*self ^ static_cast<int>(exp)), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

// Add(this: FArray, other: FArray) -> (FArray sum_state)
void STRUCTFArrayAddFArray(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<FArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  pack_farray(std::make_shared<FArray>(
      *self + std::static_pointer_cast<falcon_core::math::Quantity>(other)),
              out, oc);
}

// Add(this: FArray, other: int) -> (FArray sum_state)
void STRUCTFArrayAddInt(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm     = unpack_params(params, param_count);
  auto self   = get_opaque<FArray>(params, param_count, "this");
  int64_t val = std::get<int64_t>(pm.at("other"));
  pack_farray(std::make_shared<FArray>(*self + static_cast<int>(val)), out, oc);
}

// Add(this: FArray, other: float) -> (FArray sum_state)
void STRUCTFArrayAddFloat(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto pm    = unpack_params(params, param_count);
  auto self  = get_opaque<FArray>(params, param_count, "this");
  double val = std::get<double>(pm.at("other"));
  pack_farray(std::make_shared<FArray>(*self + val), out, oc);
}

// ── Arithmetic: Subtract ──────────────────────────────────────────────────────

// Subtract(this: FArray, other: FArray) -> (FArray difference_state)
void STRUCTFArraySubtractFArray(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto self  = get_opaque<FArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  pack_farray(std::make_shared<FArray>(
      *self - std::static_pointer_cast<falcon_core::math::Quantity>(other)),
              out, oc);
}

// Subtract(this: FArray, other: int) -> (FArray difference_state)
void STRUCTFArraySubtractInt(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto pm     = unpack_params(params, param_count);
  auto self   = get_opaque<FArray>(params, param_count, "this");
  int64_t val = std::get<int64_t>(pm.at("other"));
  pack_farray(std::make_shared<FArray>(*self - static_cast<int>(val)), out, oc);
}

// Subtract(this: FArray, other: float) -> (FArray difference_state)
void STRUCTFArraySubtractFloat(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto pm    = unpack_params(params, param_count);
  auto self  = get_opaque<FArray>(params, param_count, "this");
  double val = std::get<double>(pm.at("other"));
  pack_farray(std::make_shared<FArray>(*self - val), out, oc);
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: FArray) -> (FArray negated_state)
void STRUCTFArrayNegate(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<FArray>(params, param_count, "this");
  pack_farray(std::make_shared<FArray>(-(*self)), out, oc);
}

// Abs(this: FArray) -> (FArray absolute_state)
void STRUCTFArrayAbs(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<FArray>(params, param_count, "this");
  pack_farray(std::make_shared<FArray>(self->abs()), out, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: FArray) -> (string json)
void STRUCTFArrayToJSON(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<FArray>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (FArray state)
void STRUCTFArrayFromJSON(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto arr  = FArray::from_json_string<FArray>(json);
  pack_farray(std::make_shared<FArray>(*arr), out, oc);
}

// ── Test-only constructor (not exposed in production .fal) ────────────────────
// NewFromFloats(values: Array<float>) -> (FArray arr)
void STRUCTFArrayNewFromFloats(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto pm             = unpack_params(params, param_count);
  const auto &arr_val = std::get<ArrayValue>(pm.at("values"));
  std::vector<double> vals;
  vals.reserve(arr_val.elements.size());
  for (const auto &elem : arr_val.elements) {
    vals.push_back(std::get<double>(elem));
  }
  pack_farray(std::make_shared<FArray>(vals), out, oc);
}

} // extern "C"
