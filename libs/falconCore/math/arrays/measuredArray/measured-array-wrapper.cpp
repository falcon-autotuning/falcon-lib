#include "falcon_core/math/arrays/MeasuredArray.hpp"
#include "falcon_core/generic/FArray.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using FArray          = falcon_core::generic::FArray<double>;
using FArraySP        = std::shared_ptr<FArray>;
using MeasuredArray   = falcon_core::math::arrays::MeasuredArray;
using MeasuredArraySP = std::shared_ptr<MeasuredArray>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_ma(MeasuredArraySP ma, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "MeasuredArray";
  out[0].value.opaque.ptr       = new MeasuredArraySP(std::move(ma));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<MeasuredArraySP *>(p);
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

// New(arr: FArray) -> (MeasuredArray carray)
void STRUCTMeasuredArrayNew(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto fa = get_opaque<FArray>(params, param_count, "arr");
  pack_ma(std::make_shared<MeasuredArray>(fa), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

// Add(this: MeasuredArray, other: MeasuredArray|FArray|int|float) -> (MeasuredArray carray)
void STRUCTMeasuredArrayAdd(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ma(*self + std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ma(*self + static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "MeasuredArray") {
      auto other = std::static_pointer_cast<MeasuredArray>((*inst)->native_handle.value());
      pack_ma(*self + other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ma(*self + other, out, oc);
    }
  } else {
    throw std::runtime_error("MeasuredArray.Add: unsupported type for 'other'");
  }
}

// ── Arithmetic: Minus ─────────────────────────────────────────────────────────

// Minus(this: MeasuredArray, other: MeasuredArray|FArray|int|float) -> (MeasuredArray carray)
void STRUCTMeasuredArrayMinus(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ma(*self - std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ma(*self - static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "MeasuredArray") {
      auto other = std::static_pointer_cast<MeasuredArray>((*inst)->native_handle.value());
      pack_ma(*self - other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ma(*self - other, out, oc);
    }
  } else {
    throw std::runtime_error("MeasuredArray.Minus: unsupported type for 'other'");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: MeasuredArray) -> (MeasuredArray carray)
void STRUCTMeasuredArrayNegate(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  pack_ma(-(*self), out, oc);
}

// Abs(this: MeasuredArray) -> (MeasuredArray carray)
void STRUCTMeasuredArrayAbs(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  pack_ma(self->abs(), out, oc);
}

// ── Arithmetic: Times / Divides ───────────────────────────────────────────────

// Times(this: MeasuredArray, factor: float|int) -> (MeasuredArray carray)
void STRUCTMeasuredArrayTimes(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray>(params, param_count, "this");
  double fac;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    fac = std::get<double>(pm.at("factor"));
  } else {
    fac = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ma(*self * fac, out, oc);
}

// Divides(this: MeasuredArray, factor: float|int) -> (MeasuredArray carray)
void STRUCTMeasuredArrayDivides(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray>(params, param_count, "this");
  double div;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    div = std::get<double>(pm.at("factor"));
  } else {
    div = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ma(*self / div, out, oc);
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: MeasuredArray, exponent: float) -> (MeasuredArray carray)
void STRUCTMeasuredArrayPower(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray>(params, param_count, "this");
  double exp;
  if (std::holds_alternative<double>(pm.at("exponent"))) {
    exp = std::get<double>(pm.at("exponent"));
  } else {
    exp = static_cast<double>(std::get<int64_t>(pm.at("exponent")));
  }
  pack_ma(*self ^ exp, out, oc);
}

// ── Reduction: Min / Max ──────────────────────────────────────────────────────

// Min(this: MeasuredArray) -> (float min)
void STRUCTMeasuredArrayMin(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  pack_results(FunctionResult{self->min()}, out, 16, oc);
}

// Max(this: MeasuredArray) -> (float max)
void STRUCTMeasuredArrayMax(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  pack_results(FunctionResult{self->max()}, out, 16, oc);
}

// ── Shape manipulation ────────────────────────────────────────────────────────

// Reshape(this: MeasuredArray, shape: Array<int>) -> (MeasuredArray carray)
void STRUCTMeasuredArrayReshape(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto             pm      = unpack_params(params, param_count);
  auto             self    = get_opaque<MeasuredArray>(params, param_count, "this");
  auto             arr_val = std::get<std::shared_ptr<ArrayValue>>(pm.at("shape"));
  std::vector<size_t> shape;
  shape.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    shape.push_back(static_cast<size_t>(std::get<int64_t>(elem)));
  }
  pack_ma(self->reshape(shape), out, oc);
}

// Flip(this: MeasuredArray, axis: int) -> (MeasuredArray carray)
void STRUCTMeasuredArrayFlip(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_ma(self->flip(axis), out, oc);
}

// ── Gradient ──────────────────────────────────────────────────────────────────

// Gradient(this: MeasuredArray, axis: int) -> (FArray gradient)
void STRUCTMeasuredArrayGradient(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<MeasuredArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_farray(self->gradient(axis), out, oc);
}

// FullGradient(this: MeasuredArray) -> (Array<FArray> gradients)
void STRUCTMeasuredArrayFullGradient(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  pack_farray_list(self->gradient(), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: MeasuredArray, other: FArray) -> (bool equal)
void STRUCTMeasuredArrayEqual(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasuredArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  // Compare via the FArray base-class operator==
  bool eq = static_cast<const FArray &>(*self) == *other;
  pack_results(FunctionResult{eq}, out, 16, oc);
}

// NotEqual(this: MeasuredArray, other: FArray) -> (bool notequal)
void STRUCTMeasuredArrayNotEqual(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasuredArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  bool ne = static_cast<const FArray &>(*self) != *other;
  pack_results(FunctionResult{ne}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// FromJSON(json: string) -> (FArray farray)
void STRUCTMeasuredArrayFromJSON(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto fa   = FArray::from_json_string<FArray>(json);
  pack_farray(fa, out, oc);
}

// ToJSON(this: MeasuredArray) -> (string json)
void STRUCTMeasuredArrayToJSON(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasuredArray>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

} // extern "C"
