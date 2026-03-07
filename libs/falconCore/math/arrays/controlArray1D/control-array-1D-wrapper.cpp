#include "falcon_core/math/arrays/ControlArray1D.hpp"
#include "falcon_core/math/arrays/ControlArray.hpp"
#include "falcon_core/math/arrays/IncreasingAlignment.hpp"
#include "falcon_core/generic/FArray.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using FArray               = falcon_core::generic::FArray<double>;
using FArraySP             = std::shared_ptr<FArray>;
using ControlArray         = falcon_core::math::arrays::ControlArray;
using ControlArraySP       = std::shared_ptr<ControlArray>;
using ControlArray1D       = falcon_core::math::arrays::ControlArray1D;
using ControlArray1DSP     = std::shared_ptr<ControlArray1D>;
using IncreasingAlignment  = falcon_core::math::arrays::IncreasingAlignment;
using IncreasingAlignmentSP = std::shared_ptr<IncreasingAlignment>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_ca1d(ControlArray1DSP ca, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "ControlArray1D";
  out[0].value.opaque.ptr       = new ControlArray1DSP(std::move(ca));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ControlArray1DSP *>(p);
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

static void pack_ia(IncreasingAlignmentSP ia, FalconResultSlot *out,
                    int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "IncreasingAlignment";
  out[0].value.opaque.ptr       = new IncreasingAlignmentSP(std::move(ia));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<IncreasingAlignmentSP *>(p);
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

// New(arr: FArray) -> (ControlArray1D carray)
void STRUCTControlArray1DNew(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto fa = get_opaque<FArray>(params, param_count, "arr");
  pack_ca1d(std::make_shared<ControlArray1D>(fa), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

// Add(this: ControlArray1D, other: ControlArray|FArray|int|float) -> (ControlArray1D carray)
void STRUCTControlArray1DAdd(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ca1d(*self + std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ca1d(*self + static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "ControlArray" || (*inst)->type_name == "ControlArray1D") {
      // Wrap ControlArray in ControlArray1D before adding
      auto other_ca  = std::static_pointer_cast<ControlArray>((*inst)->native_handle.value());
      auto other_1d  = std::make_shared<ControlArray1D>(other_ca);
      pack_ca1d(*self + other_1d, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ca1d(*self + other, out, oc);
    }
  } else {
    throw std::runtime_error("ControlArray1D.Add: unsupported type for 'other'");
  }
}

// ── Arithmetic: Minus ─────────────────────────────────────────────────────────

// Minus(this: ControlArray1D, other: ControlArray1D|FArray|int|float) -> (ControlArray1D carray)
void STRUCTControlArray1DMinus(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ca1d(*self - std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ca1d(*self - static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "ControlArray1D") {
      auto other = std::static_pointer_cast<ControlArray1D>((*inst)->native_handle.value());
      pack_ca1d(*self - other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ca1d(*self - other, out, oc);
    }
  } else {
    throw std::runtime_error("ControlArray1D.Minus: unsupported type for 'other'");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: ControlArray1D) -> (ControlArray1D carray)
void STRUCTControlArray1DNegate(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_ca1d(-(*self), out, oc);
}

// Abs(this: ControlArray1D) -> (ControlArray1D carray)
void STRUCTControlArray1DAbs(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_ca1d(self->abs(), out, oc);
}

// ── Arithmetic: Times / Divides ───────────────────────────────────────────────

// Times(this: ControlArray1D, factor: float|int) -> (ControlArray1D carray)
void STRUCTControlArray1DTimes(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray1D>(params, param_count, "this");
  double fac;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    fac = std::get<double>(pm.at("factor"));
  } else {
    fac = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ca1d(*self * fac, out, oc);
}

// Divides(this: ControlArray1D, factor: float|int) -> (ControlArray1D carray)
void STRUCTControlArray1DDivides(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray1D>(params, param_count, "this");
  double div;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    div = std::get<double>(pm.at("factor"));
  } else {
    div = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ca1d(*self / div, out, oc);
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: ControlArray1D, exponent: float) -> (ControlArray1D carray)
void STRUCTControlArray1DPower(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray1D>(params, param_count, "this");
  double exp;
  if (std::holds_alternative<double>(pm.at("exponent"))) {
    exp = std::get<double>(pm.at("exponent"));
  } else {
    exp = static_cast<double>(std::get<int64_t>(pm.at("exponent")));
  }
  pack_ca1d(*self ^ exp, out, oc);
}

// ── Reduction: Min / Max ──────────────────────────────────────────────────────

// Min(this: ControlArray1D) -> (float min)
void STRUCTControlArray1DMin(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->min()}, out, 16, oc);
}

// Max(this: ControlArray1D) -> (float max)
void STRUCTControlArray1DMax(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->max()}, out, 16, oc);
}

// ── Control metadata ──────────────────────────────────────────────────────────

// PrincipleDimension(this: ControlArray1D) -> (int dim)
void STRUCTControlArray1DPrincipleDimension(const FalconParamEntry *params,
                                             int32_t param_count,
                                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->principle_dimension())},
               out, 16, oc);
}

// Alignment(this: ControlArray1D) -> (IncreasingAlignment alignment)
void STRUCTControlArray1DAlignment(const FalconParamEntry *params, int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_ia(self->alignment(), out, oc);
}

// ── 1D accessors ──────────────────────────────────────────────────────────────

// GetStart(this: ControlArray1D) -> (float start)
void STRUCTControlArray1DGetStart(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_start()}, out, 16, oc);
}

// GetEnd(this: ControlArray1D) -> (float end)
void STRUCTControlArray1DGetEnd(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_end()}, out, 16, oc);
}

// IsDecreasing(this: ControlArray1D) -> (bool decreasing)
void STRUCTControlArray1DIsDecreasing(const FalconParamEntry *params, int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->is_decreasing()}, out, 16, oc);
}

// IsIncreasing(this: ControlArray1D) -> (bool increasing)
void STRUCTControlArray1DIsIncreasing(const FalconParamEntry *params, int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->is_increasing()}, out, 16, oc);
}

// GetDistance(this: ControlArray1D) -> (float distance)
void STRUCTControlArray1DGetDistance(const FalconParamEntry *params, int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_distance()}, out, 16, oc);
}

// GetMean(this: ControlArray1D) -> (float mean)
void STRUCTControlArray1DGetMean(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_mean()}, out, 16, oc);
}

// GetSTD(this: ControlArray1D) -> (float std)
void STRUCTControlArray1DGetSTD(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_std()}, out, 16, oc);
}

// Reverse(this: ControlArray1D) -> ()
void STRUCTControlArray1DReverse(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  self->reverse();
  out[0]     = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc        = 1;
}

// GetClosestIndex(this: ControlArray1D, value: float) -> (int index)
void STRUCTControlArray1DGetClosestIndex(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto   pm    = unpack_params(params, param_count);
  auto   self  = get_opaque<ControlArray1D>(params, param_count, "this");
  double value;
  if (std::holds_alternative<double>(pm.at("value"))) {
    value = std::get<double>(pm.at("value"));
  } else {
    value = static_cast<double>(std::get<int64_t>(pm.at("value")));
  }
  size_t idx = self->get_closest_index(value);
  pack_results(FunctionResult{static_cast<int64_t>(idx)}, out, 16, oc);
}

// EvenDivisions(this: ControlArray1D, divisions: int) -> (Array<FArray> splits)
void STRUCTControlArray1DEvenDivisions(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto   pm        = unpack_params(params, param_count);
  auto   self      = get_opaque<ControlArray1D>(params, param_count, "this");
  size_t divisions = static_cast<size_t>(std::get<int64_t>(pm.at("divisions")));
  pack_farray_list(self->even_divisions(divisions), out, oc);
}

// ── Shape manipulation ────────────────────────────────────────────────────────

// Reshape(this: ControlArray1D, shape: Array<int>) -> (ControlArray1D carray)
void STRUCTControlArray1DReshape(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto             pm      = unpack_params(params, param_count);
  auto             self    = get_opaque<ControlArray1D>(params, param_count, "this");
  auto             arr_val = std::get<std::shared_ptr<ArrayValue>>(pm.at("shape"));
  std::vector<size_t> shape;
  shape.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    shape.push_back(static_cast<size_t>(std::get<int64_t>(elem)));
  }
  pack_ca1d(self->reshape(shape), out, oc);
}

// Flip(this: ControlArray1D, axis: int) -> (ControlArray1D carray)
void STRUCTControlArray1DFlip(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray1D>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_ca1d(self->flip(axis), out, oc);
}

// ── Gradient ──────────────────────────────────────────────────────────────────

// Gradient(this: ControlArray1D, axis: int) -> (FArray gradient)
void STRUCTControlArray1DGradient(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray1D>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_farray(self->gradient(axis), out, oc);
}

// FullGradient(this: ControlArray1D) -> (Array<FArray> gradients)
void STRUCTControlArray1DFullGradient(const FalconParamEntry *params,
                                       int32_t param_count, FalconResultSlot *out,
                                       int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_farray_list(self->gradient(), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: ControlArray1D, other: ControlArray1D) -> (bool equal)
void STRUCTControlArray1DEqual(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<ControlArray1D>(params, param_count, "this");
  auto other = get_opaque<ControlArray1D>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: ControlArray1D, other: ControlArray1D) -> (bool notequal)
void STRUCTControlArray1DNotEqual(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<ControlArray1D>(params, param_count, "this");
  auto other = get_opaque<ControlArray1D>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// FromJSON(json: string) -> (ControlArray1D carray)
void STRUCTControlArray1DFromJSON(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto ca   = ControlArray1D::from_json_string<ControlArray1D>(json);
  pack_ca1d(ca, out, oc);
}

// ToJSON(this: ControlArray1D) -> (string json)
void STRUCTControlArray1DToJSON(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

} // extern "C"
