#include "falcon_core/math/arrays/LabelledMeasuredArray.hpp"
#include "falcon_core/math/arrays/MeasuredArray.hpp"
#include "falcon_core/generic/FArray.hpp"
#include "falcon_core/autotuner_interfaces/contexts/AcquisitionContext.hpp"
#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using FArray                  = falcon_core::generic::FArray<double>;
using FArraySP                = std::shared_ptr<FArray>;
using MeasuredArray           = falcon_core::math::arrays::MeasuredArray;
using MeasuredArraySP         = std::shared_ptr<MeasuredArray>;
using LabelledMeasuredArray   = falcon_core::math::arrays::LabelledMeasuredArray;
using LabelledMeasuredArraySP = std::shared_ptr<LabelledMeasuredArray>;
using AcquisitionContext      = falcon_core::autotuner_interfaces::contexts::AcquisitionContext;
using AcquisitionContextSP    = std::shared_ptr<AcquisitionContext>;
using InstrumentPort          = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP        = std::shared_ptr<InstrumentPort>;
using Connection              = falcon_core::physics::device_structures::Connection;
using ConnectionSP            = std::shared_ptr<Connection>;
using SymbolUnit              = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP            = std::shared_ptr<SymbolUnit>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_lma(LabelledMeasuredArraySP lma, FalconResultSlot *out,
                     int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledMeasuredArray";
  out[0].value.opaque.ptr       = new LabelledMeasuredArraySP(std::move(lma));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<LabelledMeasuredArraySP *>(p);
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

// New(arr: FArray) -> (LabelledMeasuredArray carray)
void STRUCTLabelledMeasuredArrayNew(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto fa   = get_opaque<FArray>(params, param_count, "arr");
  auto port = make_default_port();
  pack_lma(std::make_shared<LabelledMeasuredArray>(fa, port), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayAdd(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_lma(*self + std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_lma(*self + static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    const std::string &tn = (*inst)->type_name;
    if (tn == "LabelledMeasuredArray") {
      auto other = std::static_pointer_cast<LabelledMeasuredArray>((*inst)->native_handle.value());
      pack_lma(*self + other, out, oc);
    } else if (tn == "MeasuredArray") {
      auto other = std::static_pointer_cast<MeasuredArray>((*inst)->native_handle.value());
      auto fa    = std::static_pointer_cast<FArray>(other);
      pack_lma(*self + fa, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_lma(*self + other, out, oc);
    }
  } else {
    throw std::runtime_error("LabelledMeasuredArray.Add: unsupported type");
  }
}

// ── Arithmetic: Minus ─────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayMinus(const FalconParamEntry *params,
                                       int32_t param_count, FalconResultSlot *out,
                                       int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_lma(*self - std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_lma(*self - static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    const std::string &tn = (*inst)->type_name;
    if (tn == "LabelledMeasuredArray") {
      auto other = std::static_pointer_cast<LabelledMeasuredArray>((*inst)->native_handle.value());
      pack_lma(*self - other, out, oc);
    } else if (tn == "MeasuredArray") {
      auto other = std::static_pointer_cast<MeasuredArray>((*inst)->native_handle.value());
      auto fa    = std::static_pointer_cast<FArray>(other);
      pack_lma(*self - fa, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_lma(*self - other, out, oc);
    }
  } else {
    throw std::runtime_error("LabelledMeasuredArray.Minus: unsupported type");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayNegate(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  pack_lma(-(*self), out, oc);
}

void STRUCTLabelledMeasuredArrayAbs(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  pack_lma(self->abs(), out, oc);
}

// ── Times / Divides / Power ───────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayTimes(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  double fac  = std::holds_alternative<double>(pm.at("factor"))
                    ? std::get<double>(pm.at("factor"))
                    : static_cast<double>(std::get<int64_t>(pm.at("factor")));
  pack_lma(*self * fac, out, oc);
}

void STRUCTLabelledMeasuredArrayDivides(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  double div  = std::holds_alternative<double>(pm.at("factor"))
                    ? std::get<double>(pm.at("factor"))
                    : static_cast<double>(std::get<int64_t>(pm.at("factor")));
  pack_lma(*self / div, out, oc);
}

void STRUCTLabelledMeasuredArrayPower(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  double exp  = std::holds_alternative<double>(pm.at("exponent"))
                    ? std::get<double>(pm.at("exponent"))
                    : static_cast<double>(std::get<int64_t>(pm.at("exponent")));
  pack_lma(*self ^ exp, out, oc);
}

// ── Reduction ─────────────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayMin(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  pack_results(FunctionResult{self->min()}, out, 16, oc);
}

void STRUCTLabelledMeasuredArrayMax(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  pack_results(FunctionResult{self->max()}, out, 16, oc);
}

// ── Label metadata ────────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayLabel(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  pack_ac(self->label(), out, oc);
}

void STRUCTLabelledMeasuredArrayConnection(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
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

void STRUCTLabelledMeasuredArrayInstrumentType(const FalconParamEntry *params,
                                                int32_t param_count,
                                                FalconResultSlot *out,
                                                int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  auto inst = self->label()->instrument_type();
  pack_results(FunctionResult{inst}, out, 16, oc);
}

void STRUCTLabelledMeasuredArrayUnits(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto  self  = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  auto  units = self->label()->units();
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

void STRUCTLabelledMeasuredArrayReshape(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto             pm      = unpack_params(params, param_count);
  auto             self    = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  auto             arr_val = std::get<std::shared_ptr<ArrayValue>>(pm.at("shape"));
  std::vector<size_t> shape;
  for (const auto &elem : arr_val->elements)
    shape.push_back(static_cast<size_t>(std::get<int64_t>(elem)));
  pack_lma(self->reshape(shape), out, oc);
}

void STRUCTLabelledMeasuredArrayFlip(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_lma(self->flip(axis), out, oc);
}

// ── Gradient ──────────────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayGradient(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_farray(self->gradient(axis), out, oc);
}

void STRUCTLabelledMeasuredArrayFullGradient(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  pack_farray_list(self->gradient(), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  bool eq    = static_cast<const FArray &>(*self) == *other;
  pack_results(FunctionResult{eq}, out, 16, oc);
}

void STRUCTLabelledMeasuredArrayNotEqual(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  auto other = get_opaque<FArray>(params, param_count, "other");
  bool ne    = static_cast<const FArray &>(*self) != *other;
  pack_results(FunctionResult{ne}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

void STRUCTLabelledMeasuredArrayToJSON(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledMeasuredArray>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTLabelledMeasuredArrayFromJSON(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto fa   = FArray::from_json_string<FArray>(json);
  pack_farray(fa, out, oc);
}

} // extern "C"
