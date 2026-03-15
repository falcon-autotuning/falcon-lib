#include "falcon_core/math/arrays/LabelledControlArray.hpp"
#include "falcon_core/math/arrays/ControlArray.hpp"
#include "falcon_core/math/arrays/IncreasingAlignment.hpp"
#include "falcon_core/generic/FArray.hpp"
#include "falcon_core/autotuner_interfaces/contexts/AcquisitionContext.hpp"
#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using FArray               = falcon_core::generic::FArray<double>;
using FArraySP             = std::shared_ptr<FArray>;
using ControlArray         = falcon_core::math::arrays::ControlArray;
using ControlArraySP       = std::shared_ptr<ControlArray>;
using LabelledControlArray    = falcon_core::math::arrays::LabelledControlArray;
using LabelledControlArraySP  = std::shared_ptr<LabelledControlArray>;
using IncreasingAlignment     = falcon_core::math::arrays::IncreasingAlignment;
using IncreasingAlignmentSP   = std::shared_ptr<IncreasingAlignment>;
using AcquisitionContext      = falcon_core::autotuner_interfaces::contexts::AcquisitionContext;
using AcquisitionContextSP    = std::shared_ptr<AcquisitionContext>;
using InstrumentPort          = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP        = std::shared_ptr<InstrumentPort>;
using Connection              = falcon_core::physics::device_structures::Connection;
using ConnectionSP            = std::shared_ptr<Connection>;
using SymbolUnit              = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP            = std::shared_ptr<SymbolUnit>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_lca(LabelledControlArraySP lca, FalconResultSlot *out,
                     int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledControlArray";
  out[0].value.opaque.ptr       = new LabelledControlArraySP(std::move(lca));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<LabelledControlArraySP *>(p);
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

static void pack_ac(AcquisitionContextSP ac, FalconResultSlot *out,
                    int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "AcquisitionContext";
  out[0].value.opaque.ptr       = new AcquisitionContextSP(std::move(ac));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<AcquisitionContextSP *>(p);
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

// Create a minimal InstrumentPort for use when only an FArray is given.
static InstrumentPortSP make_default_port() {
  return std::make_shared<InstrumentPort>("default");
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(arr: FArray) -> (LabelledControlArray carray)
void STRUCTLabelledControlArrayNew(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto fa   = get_opaque<FArray>(params, param_count, "arr");
  auto port = make_default_port();
  pack_lca(std::make_shared<LabelledControlArray>(fa, port), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayAdd(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_lca(*self + std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_lca(*self + static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    const std::string &tn = (*inst)->type_name;
    if (tn == "LabelledControlArray") {
      auto other = std::static_pointer_cast<LabelledControlArray>((*inst)->native_handle.value());
      pack_lca(*self + other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_lca(*self + other, out, oc);
    }
  } else {
    throw std::runtime_error("LabelledControlArray.Add: unsupported type");
  }
}

// ── Arithmetic: Minus ─────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayMinus(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_lca(*self - std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_lca(*self - static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    const std::string &tn = (*inst)->type_name;
    if (tn == "LabelledControlArray") {
      auto other = std::static_pointer_cast<LabelledControlArray>((*inst)->native_handle.value());
      pack_lca(*self - other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_lca(*self - other, out, oc);
    }
  } else {
    throw std::runtime_error("LabelledControlArray.Minus: unsupported type");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayNegate(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_lca(-(*self), out, oc);
}

void STRUCTLabelledControlArrayAbs(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_lca(self->abs(), out, oc);
}

// ── Times / Divides / Power ───────────────────────────────────────────────────

void STRUCTLabelledControlArrayTimes(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray>(params, param_count, "this");
  double fac  = std::holds_alternative<double>(pm.at("factor"))
                    ? std::get<double>(pm.at("factor"))
                    : static_cast<double>(std::get<int64_t>(pm.at("factor")));
  pack_lca(*self * fac, out, oc);
}

void STRUCTLabelledControlArrayDivides(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray>(params, param_count, "this");
  double div  = std::holds_alternative<double>(pm.at("factor"))
                    ? std::get<double>(pm.at("factor"))
                    : static_cast<double>(std::get<int64_t>(pm.at("factor")));
  pack_lca(*self / div, out, oc);
}

void STRUCTLabelledControlArrayPower(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray>(params, param_count, "this");
  double exp  = std::holds_alternative<double>(pm.at("exponent"))
                    ? std::get<double>(pm.at("exponent"))
                    : static_cast<double>(std::get<int64_t>(pm.at("exponent")));
  pack_lca(*self ^ exp, out, oc);
}

// ── Reduction ─────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayMin(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_results(FunctionResult{self->min()}, out, 16, oc);
}

void STRUCTLabelledControlArrayMax(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_results(FunctionResult{self->max()}, out, 16, oc);
}

// ── Metadata ─────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayPrincipleDimension(
    const FalconParamEntry *params, int32_t param_count, FalconResultSlot *out,
    int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_results(
      FunctionResult{static_cast<int64_t>(self->principle_dimension())}, out,
      16, oc);
}

void STRUCTLabelledControlArrayAlignment(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_ia(self->alignment(), out, oc);
}

void STRUCTLabelledControlArrayLabel(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_ac(self->label(), out, oc);
}

void STRUCTLabelledControlArrayConnection(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  auto conn = self->label()->connection();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr       = new ConnectionSP(conn);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

void STRUCTLabelledControlArrayInstrumentType(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  auto inst = self->label()->instrument_type();
  pack_results(FunctionResult{inst}, out, 16, oc);
}

void STRUCTLabelledControlArrayUnits(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledControlArray>(params, param_count, "this");
  auto units = self->label()->units();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr       = new SymbolUnitSP(units);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// ── Shape ─────────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayReshape(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto             pm      = unpack_params(params, param_count);
  auto             self    = get_opaque<LabelledControlArray>(params, param_count, "this");
  auto             arr_val = std::get<std::shared_ptr<ArrayValue>>(pm.at("shape"));
  std::vector<size_t> shape;
  for (const auto &elem : arr_val->elements)
    shape.push_back(static_cast<size_t>(std::get<int64_t>(elem)));
  pack_lca(self->reshape(shape), out, oc);
}

void STRUCTLabelledControlArrayFlip(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_lca(self->flip(axis), out, oc);
}

// ── Gradient ──────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayGradient(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_farray(self->gradient(axis), out, oc);
}

void STRUCTLabelledControlArrayFullGradient(const FalconParamEntry *params,
                                             int32_t param_count,
                                             FalconResultSlot *out,
                                             int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_farray_list(self->gradient(), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayEqual(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledControlArray>(params, param_count, "this");
  auto other = get_opaque<LabelledControlArray>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTLabelledControlArrayNotEqual(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledControlArray>(params, param_count, "this");
  auto other = get_opaque<LabelledControlArray>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArrayToJSON(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTLabelledControlArrayFromJSON(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto lca  = LabelledControlArray::from_json_string<LabelledControlArray>(json);
  pack_lca(lca, out, oc);
}

} // extern "C"
