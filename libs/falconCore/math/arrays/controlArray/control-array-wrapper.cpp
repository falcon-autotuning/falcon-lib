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
using IncreasingAlignment  = falcon_core::math::arrays::IncreasingAlignment;
using IncreasingAlignmentSP = std::shared_ptr<IncreasingAlignment>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_ca(ControlArraySP ca, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "ControlArray";
  out[0].value.opaque.ptr       = new ControlArraySP(std::move(ca));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ControlArraySP *>(p);
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
    auto inst          = std::make_shared<StructInstance>("FArray");
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

// New(arr: FArray) -> (ControlArray carray)
void STRUCTControlArrayNew(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto fa = get_opaque<FArray>(params, param_count, "arr");
  pack_ca(std::make_shared<ControlArray>(fa), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

// Add(this: ControlArray, other: ControlArray|FArray|int|float) -> (ControlArray carray)
void STRUCTControlArrayAdd(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ca(*self + std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ca(*self + static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "ControlArray") {
      auto other = std::static_pointer_cast<ControlArray>((*inst)->native_handle.value());
      pack_ca(*self + other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ca(*self + other, out, oc);
    }
  } else {
    throw std::runtime_error("ControlArray.Add: unsupported type for 'other'");
  }
}

// ── Arithmetic: Minus ─────────────────────────────────────────────────────────

// Minus(this: ControlArray, other: ControlArray|FArray|int|float) -> (ControlArray carray)
void STRUCTControlArrayMinus(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_ca(*self - std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_ca(*self - static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    if ((*inst)->type_name == "ControlArray") {
      auto other = std::static_pointer_cast<ControlArray>((*inst)->native_handle.value());
      pack_ca(*self - other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_ca(*self - other, out, oc);
    }
  } else {
    throw std::runtime_error("ControlArray.Minus: unsupported type for 'other'");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: ControlArray) -> (ControlArray carray)
void STRUCTControlArrayNegate(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_ca(-(*self), out, oc);
}

// Abs(this: ControlArray) -> (ControlArray carray)
void STRUCTControlArrayAbs(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_ca(self->abs(), out, oc);
}

// ── Arithmetic: Times / Divides ───────────────────────────────────────────────

// Times(this: ControlArray, factor: float|int) -> (ControlArray carray)
void STRUCTControlArrayTimes(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray>(params, param_count, "this");
  double fac;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    fac = std::get<double>(pm.at("factor"));
  } else {
    fac = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ca(*self * fac, out, oc);
}

// Divides(this: ControlArray, factor: float|int) -> (ControlArray carray)
void STRUCTControlArrayDivides(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray>(params, param_count, "this");
  double div;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    div = std::get<double>(pm.at("factor"));
  } else {
    div = static_cast<double>(std::get<int64_t>(pm.at("factor")));
  }
  pack_ca(*self / div, out, oc);
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: ControlArray, exponent: float) -> (ControlArray carray)
void STRUCTControlArrayPower(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray>(params, param_count, "this");
  double exp;
  if (std::holds_alternative<double>(pm.at("exponent"))) {
    exp = std::get<double>(pm.at("exponent"));
  } else {
    exp = static_cast<double>(std::get<int64_t>(pm.at("exponent")));
  }
  pack_ca(*self ^ exp, out, oc);
}

// ── Reduction: Min / Max ──────────────────────────────────────────────────────

// Min(this: ControlArray) -> (float min)
void STRUCTControlArrayMin(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_results(FunctionResult{self->min()}, out, 16, oc);
}

// Max(this: ControlArray) -> (float max)
void STRUCTControlArrayMax(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_results(FunctionResult{self->max()}, out, 16, oc);
}

// ── Control metadata ──────────────────────────────────────────────────────────

// PrincipleDimension(this: ControlArray) -> (int dim)
void STRUCTControlArrayPrincipleDimension(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->principle_dimension())},
               out, 16, oc);
}

// Alignment(this: ControlArray) -> (IncreasingAlignment alignment)
void STRUCTControlArrayAlignment(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_ia(self->alignment(), out, oc);
}

// ── Shape manipulation ────────────────────────────────────────────────────────

// Reshape(this: ControlArray, shape: Array<int>) -> (ControlArray carray)
void STRUCTControlArrayReshape(const FalconParamEntry *params, int32_t param_count,
                                FalconResultSlot *out, int32_t *oc) {
  auto             pm      = unpack_params(params, param_count);
  auto             self    = get_opaque<ControlArray>(params, param_count, "this");
  auto             arr_val = std::get<std::shared_ptr<ArrayValue>>(pm.at("shape"));
  std::vector<size_t> shape;
  shape.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    shape.push_back(static_cast<size_t>(std::get<int64_t>(elem)));
  }
  pack_ca(self->reshape(shape), out, oc);
}

// Flip(this: ControlArray, axis: int) -> (ControlArray carray)
void STRUCTControlArrayFlip(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_ca(self->flip(axis), out, oc);
}

// ── Gradient ──────────────────────────────────────────────────────────────────

// Gradient(this: ControlArray, axis: int) -> (FArray gradient)
void STRUCTControlArrayGradient(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<ControlArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_farray(self->gradient(axis), out, oc);
}

// FullGradient(this: ControlArray) -> (Array<FArray> gradients)
void STRUCTControlArrayFullGradient(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_farray_list(self->gradient(), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: ControlArray, other: ControlArray) -> (bool equal)
void STRUCTControlArrayEqual(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<ControlArray>(params, param_count, "this");
  auto other = get_opaque<ControlArray>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: ControlArray, other: ControlArray) -> (bool notequal)
void STRUCTControlArrayNotEqual(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<ControlArray>(params, param_count, "this");
  auto other = get_opaque<ControlArray>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// FromJSON(json: string) -> (ControlArray carray)
void STRUCTControlArrayFromJSON(const FalconParamEntry *params, int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto ca   = ControlArray::from_json_string<ControlArray>(json);
  pack_ca(ca, out, oc);
}

// ToJSON(this: ControlArray) -> (string json)
void STRUCTControlArrayToJSON(const FalconParamEntry *params, int32_t param_count,
                               FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<ControlArray>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

} // extern "C"
