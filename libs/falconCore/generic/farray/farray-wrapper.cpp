#include "falcon_core/generic/FArray.hpp"
#include <xtensor/xadapt.hpp>
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// FArray<T> is a C++ template. The Falcon DSL binding is always over doubles
// (the optimiser pipeline operates in double precision). Monomorphize here.
using FArray   = falcon_core::generic::FArray<double>;
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
// Times(this: FArray, factor: int|float) -> (FArray scaled_state)
void STRUCTFArrayTimes(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<FArray>(params, param_count, "this");
  double fac;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    fac = std::get<double>(pm.at("factor"));
  } else if (std::holds_alternative<int64_t>(pm.at("factor"))) {
    fac = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  } else {
    throw std::runtime_error("factor must be int or float");
  }
  pack_farray(*self * fac, out, oc);
}

// ── Arithmetic: Divides ───────────────────────────────────────────────────────
// Divides(this: FArray, divisor: int|float) -> (FArray scaled_state)
void STRUCTFArrayDivides(const FalconParamEntry *params,
                         int32_t param_count, FalconResultSlot *out,
                         int32_t *oc) {
  auto pm    = unpack_params(params, param_count);
  auto self  = get_opaque<FArray>(params, param_count, "this");
  double div;
  if (std::holds_alternative<double>(pm.at("divisor"))) {
    div = std::get<double>(pm.at("divisor"));
  } else if (std::holds_alternative<int64_t>(pm.at("divisor"))) {
    div = static_cast<double>(std::get<int64_t>(pm.at("divisor")));
  } else {
    throw std::runtime_error("divisor must be int or float");
  }
  pack_farray(*self / div, out, oc);
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: FArray, exponent: float) -> (FArray powered_state)
void STRUCTFArrayPower(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto pm     = unpack_params(params, param_count);
  auto self   = get_opaque<FArray>(params, param_count, "this");
  int64_t exp = std::get<int64_t>(pm.at("exponent"));
  pack_farray(*self ^ static_cast<int>(exp), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────
// Add(this: FArray, other: FArray|int|float) -> (FArray sum_state)
void STRUCTFArrayAdd(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<FArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    double val = std::get<double>(pm.at("other"));
    pack_farray(*self + val, out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    int64_t val = std::get<int64_t>(pm.at("other"));
    pack_farray(*self + static_cast<double>(val), out, oc);
  } else if (auto other = get_opaque<FArray>(params, param_count, "other")) {
    pack_farray(*self + other, out, oc);
  } else {
    throw std::runtime_error("other must be FArray, int, or float");
  }
}

// ── Arithmetic: Subtract ──────────────────────────────────────────────────────
// Subtract(this: FArray, other: FArray|int|float) -> (FArray difference_state)
void STRUCTFArraySubtract(const FalconParamEntry *params,
                         int32_t param_count, FalconResultSlot *out,
                         int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<FArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    double val = std::get<double>(pm.at("other"));
    pack_farray(*self - val, out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    int64_t val = std::get<int64_t>(pm.at("other"));
    pack_farray(*self - static_cast<double>(val), out, oc);
  } else if (auto other = get_opaque<FArray>(params, param_count, "other")) {
    pack_farray(*self - other, out, oc);
  } else {
    throw std::runtime_error("other must be FArray, int, or float");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: FArray) -> (FArray negated_state)
void STRUCTFArrayNegate(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<FArray>(params, param_count, "this");
  pack_farray(-(*self), out, oc);
}

// Abs(this: FArray) -> (FArray absolute_state)
void STRUCTFArrayAbs(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<FArray>(params, param_count, "this");
  pack_farray(self->abs(), out, oc);
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
  pack_farray(arr, out, oc);
}

// ── Test-only constructor ─────────────────────────────────────────────────────
// NewFromFloats(values: Array<float>) -> (FArray arr)
// Not exposed in the production .fal — only imported by tests/farray-test-helpers.fal
void STRUCTFArrayTestHelpersNewFromFloats(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto pm             = unpack_params(params, param_count);
  auto arr_val        = std::get<std::shared_ptr<ArrayValue>>(pm.at("values"));
  std::vector<double> vals;
  vals.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    vals.push_back(std::get<double>(elem));
  }
  pack_farray(std::make_shared<FArray>(xt::adapt(vals)), out, oc);
}

} // extern "C"
