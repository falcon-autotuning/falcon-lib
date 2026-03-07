#include "falcon_core/math/arrays/MeasuredArray1D.hpp"
#include "falcon_core/math/arrays/MeasuredArray.hpp"
#include "falcon_core/generic/FArray.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using FArray             = falcon_core::generic::FArray<double>;
using FArraySP           = std::shared_ptr<FArray>;
using MeasuredArray      = falcon_core::math::arrays::MeasuredArray;
using MeasuredArraySP    = std::shared_ptr<MeasuredArray>;
using MeasuredArray1D    = falcon_core::math::arrays::MeasuredArray1D;
using MeasuredArray1DSP  = std::shared_ptr<MeasuredArray1D>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_ma1d(MeasuredArray1DSP ma, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "MeasuredArray1D";
  out[0].value.opaque.ptr       = new MeasuredArray1DSP(std::move(ma));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<MeasuredArray1DSP *>(p);
  };
  *oc = 1;
}

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

static void pack_farray_list(
    falcon_core::generic::ListSP<FArray> grads,
    FalconResultSlot *out, int32_t *oc) {
  auto arr = std::make_shared<ArrayValue>("FArray");
  for (const auto &g : grads->items()) {
    auto inst           = std::make_shared<StructInstance>("FArray");
    inst->native_handle = std::static_pointer_cast<void>(g);
    arr->elements.push_back(std::move(inst));
  }
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "ArrayValue";
  out[0].value.opaque.ptr       = new std::shared_ptr<ArrayValue>(arr);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<std::shared_ptr<ArrayValue> *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(arr: FArray) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DNew(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto fa = get_opaque<FArray>(params, param_count, "arr");
  pack_ma1d(std::make_shared<MeasuredArray1D>(fa), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

// Add(this: MeasuredArray1D, other: MeasuredArray|FArray|int|float) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DAdd(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ma1d(*self + std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ma1d(*self + static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "MeasuredArray" || (*inst)->type_name == "MeasuredArray1D") {
      // Wrap MeasuredArray in MeasuredArray1D before adding
      auto other_ma  = std::static_pointer_cast<MeasuredArray>((*inst)->native_handle.value());
      auto other_1d  = std::make_shared<MeasuredArray1D>(other_ma);
      pack_ma1d(*self + other_1d, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ma1d(*self + other, out, oc);
    }
  } else {
    throw std::runtime_error("MeasuredArray1D.Add: unsupported type for 'other'");
  }
}

// ── Arithmetic: Minus ─────────────────────────────────────────────────────────

// Minus(this: MeasuredArray1D, other: MeasuredArray|FArray|int|float) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DMinus(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ma1d(*self - std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ma1d(*self - static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "MeasuredArray" || (*inst)->type_name == "MeasuredArray1D") {
      auto other_ma  = std::static_pointer_cast<MeasuredArray>((*inst)->native_handle.value());
      auto other_1d  = std::make_shared<MeasuredArray1D>(other_ma);
      pack_ma1d(*self - other_1d, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ma1d(*self - other, out, oc);
    }
  } else {
    throw std::runtime_error("MeasuredArray1D.Minus: unsupported type for 'other'");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: MeasuredArray1D) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DNegate(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_ma1d(-(*self), out, oc);
}

// Abs(this: MeasuredArray1D) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DAbs(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_ma1d(self->abs(), out, oc);
}

// ── Arithmetic: Times / Divides ───────────────────────────────────────────────

// Times(this: MeasuredArray1D, factor: float|int) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DTimes(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  double fac;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    fac = std::get<double>(pm.at("factor"));
  } else {
    fac = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ma1d(*self * fac, out, oc);
}

// Divides(this: MeasuredArray1D, factor: float|int) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DDivides(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  double div;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    div = std::get<double>(pm.at("factor"));
  } else {
    div = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ma1d(*self / div, out, oc);
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: MeasuredArray1D, exponent: float) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DPower(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  double exp;
  if (std::holds_alternative<double>(pm.at("exponent"))) {
    exp = std::get<double>(pm.at("exponent"));
  } else {
    exp = static_cast<double>(std::get<int64_t>(pm.at("exponent")));
  }
  pack_ma1d(*self ^ exp, out, oc);
}

// ── Reduction: Min / Max ──────────────────────────────────────────────────────

// Min(this: MeasuredArray1D) -> (float min)
void STRUCTMeasuredArray1DMin(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->min()}, out, 16, oc);
}

// Max(this: MeasuredArray1D) -> (float max)
void STRUCTMeasuredArray1DMax(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->max()}, out, 16, oc);
}

// ── 1D accessors ──────────────────────────────────────────────────────────────

// GetStart(this: MeasuredArray1D) -> (float start)
void STRUCTMeasuredArray1DGetStart(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_start()}, out, 16, oc);
}

// GetEnd(this: MeasuredArray1D) -> (float end)
void STRUCTMeasuredArray1DGetEnd(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_end()}, out, 16, oc);
}

// IsDecreasing(this: MeasuredArray1D) -> (bool decreasing)
void STRUCTMeasuredArray1DIsDecreasing(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->is_decreasing()}, out, 16, oc);
}

// IsIncreasing(this: MeasuredArray1D) -> (bool increasing)
void STRUCTMeasuredArray1DIsIncreasing(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->is_increasing()}, out, 16, oc);
}

// GetDistance(this: MeasuredArray1D) -> (float distance)
void STRUCTMeasuredArray1DGetDistance(const FalconParamEntry *params, int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_distance()}, out, 16, oc);
}

// GetMean(this: MeasuredArray1D) -> (float mean)
void STRUCTMeasuredArray1DGetMean(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_mean()}, out, 16, oc);
}

// GetSTD(this: MeasuredArray1D) -> (float std)
void STRUCTMeasuredArray1DGetSTD(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_std()}, out, 16, oc);
}

// Reverse(this: MeasuredArray1D) -> ()
void STRUCTMeasuredArray1DReverse(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  self->reverse();
  out[0]     = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc        = 1;
}

// GetClosestIndex(this: MeasuredArray1D, value: float) -> (int index)
void STRUCTMeasuredArray1DGetClosestIndex(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto   pm    = unpack_params(params, param_count);
  auto   self  = get_opaque<MeasuredArray1D>(params, param_count, "this");
  double value;
  if (std::holds_alternative<double>(pm.at("value"))) {
    value = std::get<double>(pm.at("value"));
  } else {
    value = static_cast<double>(std::get<int64_t>(pm.at("value")));
  }
  size_t idx = self->get_closest_index(value);
  pack_results(FunctionResult{static_cast<int64_t>(idx)}, out, 16, oc);
}

// EvenDivisions(this: MeasuredArray1D, divisions: int) -> (Array<FArray> splits)
void STRUCTMeasuredArray1DEvenDivisions(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto   pm        = unpack_params(params, param_count);
  auto   self      = get_opaque<MeasuredArray1D>(params, param_count, "this");
  size_t divisions = static_cast<size_t>(std::get<int64_t>(pm.at("divisions")));
  pack_farray_list(self->even_divisions(divisions), out, oc);
}

// ── Shape manipulation ────────────────────────────────────────────────────────

// Reshape(this: MeasuredArray1D, shape: Array<int>) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DReshape(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto             pm      = unpack_params(params, param_count);
  auto             self    = get_opaque<MeasuredArray1D>(params, param_count, "this");
  auto             arr_val = std::get<std::shared_ptr<ArrayValue>>(pm.at("shape"));
  std::vector<size_t> shape;
  shape.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    shape.push_back(static_cast<size_t>(std::get<int64_t>(elem)));
  }
  pack_ma1d(self->reshape(shape), out, oc);
}

// Flip(this: MeasuredArray1D, axis: int) -> (MeasuredArray1D carray)
void STRUCTMeasuredArray1DFlip(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_ma1d(self->flip(axis), out, oc);
}

// ── Gradient ──────────────────────────────────────────────────────────────────

// Gradient(this: MeasuredArray1D, axis: int) -> (FArray gradient)
void STRUCTMeasuredArray1DGradient(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_farray(self->gradient(axis), out, oc);
}

// FullGradient(this: MeasuredArray1D) -> (Array<FArray> gradients)
void STRUCTMeasuredArray1DFullGradient(const FalconParamEntry *params,
                                        int32_t param_count, FalconResultSlot *out,
                                        int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_farray_list(self->gradient(), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: MeasuredArray1D, other: MeasuredArray1D) -> (bool equal)
void STRUCTMeasuredArray1DEqual(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasuredArray1D>(params, param_count, "this");
  auto other = get_opaque<MeasuredArray1D>(params, param_count, "other");
  // MeasuredArray1D inherits MeasuredArray::operator==(const MeasuredArray&)
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: MeasuredArray1D, other: MeasuredArray1D) -> (bool notequal)
void STRUCTMeasuredArray1DNotEqual(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasuredArray1D>(params, param_count, "this");
  auto other = get_opaque<MeasuredArray1D>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// FromJSON(json: string) -> (MeasuredArray1D farray)
void STRUCTMeasuredArray1DFromJSON(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto ma   = MeasuredArray1D::from_json_string<MeasuredArray1D>(json);
  pack_ma1d(ma, out, oc);
}

// ToJSON(this: MeasuredArray1D) -> (string json)
void STRUCTMeasuredArray1DToJSON(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

} // extern "C"
