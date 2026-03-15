#include "falcon_core/math/arrays/LabelledControlArray1D.hpp"
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

using FArray                    = falcon_core::generic::FArray<double>;
using FArraySP                  = std::shared_ptr<FArray>;
using LabelledControlArray1D    = falcon_core::math::arrays::LabelledControlArray1D;
using LabelledControlArray1DSP  = std::shared_ptr<LabelledControlArray1D>;
using IncreasingAlignment       = falcon_core::math::arrays::IncreasingAlignment;
using IncreasingAlignmentSP     = std::shared_ptr<IncreasingAlignment>;
using AcquisitionContext        = falcon_core::autotuner_interfaces::contexts::AcquisitionContext;
using AcquisitionContextSP      = std::shared_ptr<AcquisitionContext>;
using InstrumentPort            = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP          = std::shared_ptr<InstrumentPort>;
using Connection                = falcon_core::physics::device_structures::Connection;
using ConnectionSP              = std::shared_ptr<Connection>;
using SymbolUnit                = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP              = std::shared_ptr<SymbolUnit>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_lca1d(LabelledControlArray1DSP lca, FalconResultSlot *out,
                       int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledControlArray1D";
  out[0].value.opaque.ptr       = new LabelledControlArray1DSP(std::move(lca));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<LabelledControlArray1DSP *>(p);
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

static InstrumentPortSP make_default_port() {
  return std::make_shared<InstrumentPort>("default");
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(arr: FArray) -> (LabelledControlArray1D carray)
void STRUCTLabelledControlArray1DNew(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto fa   = get_opaque<FArray>(params, param_count, "arr");
  auto port = make_default_port();
  auto ac   = std::make_shared<AcquisitionContext>(port);
  pack_lca1d(std::make_shared<LabelledControlArray1D>(fa, ac), out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DAdd(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_lca1d(*self + std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_lca1d(*self + static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    const std::string &tn = (*inst)->type_name;
    if (tn == "LabelledControlArray1D") {
      auto other = std::static_pointer_cast<LabelledControlArray1D>((*inst)->native_handle.value());
      pack_lca1d(*self + other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_lca1d(*self + other, out, oc);
    }
  } else {
    throw std::runtime_error("LabelledControlArray1D.Add: unsupported type");
  }
}

// ── Arithmetic: Minus ─────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DMinus(const FalconParamEntry *params,
                                        int32_t param_count, FalconResultSlot *out,
                                        int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("other"))) {
    pack_lca1d(*self - std::get<double>(pm.at("other")), out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    pack_lca1d(*self - static_cast<int>(std::get<int64_t>(pm.at("other"))), out, oc);
  } else if (auto inst = std::get_if<std::shared_ptr<StructInstance>>(&pm.at("other"))) {
    const std::string &tn = (*inst)->type_name;
    if (tn == "LabelledControlArray1D") {
      auto other = std::static_pointer_cast<LabelledControlArray1D>((*inst)->native_handle.value());
      pack_lca1d(*self - other, out, oc);
    } else {
      auto other = std::static_pointer_cast<FArray>((*inst)->native_handle.value());
      pack_lca1d(*self - other, out, oc);
    }
  } else {
    throw std::runtime_error("LabelledControlArray1D.Minus: unsupported type");
  }
}

// ── Unary ─────────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DNegate(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_lca1d(-(*self), out, oc);
}

void STRUCTLabelledControlArray1DAbs(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_lca1d(self->abs(), out, oc);
}

// ── Times / Divides / Power ───────────────────────────────────────────────────

void STRUCTLabelledControlArray1DTimes(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  double fac  = std::holds_alternative<double>(pm.at("factor"))
                    ? std::get<double>(pm.at("factor"))
                    : static_cast<double>(std::get<int64_t>(pm.at("factor")));
  pack_lca1d(*self * fac, out, oc);
}

void STRUCTLabelledControlArray1DDivides(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  double div  = std::holds_alternative<double>(pm.at("factor"))
                    ? std::get<double>(pm.at("factor"))
                    : static_cast<double>(std::get<int64_t>(pm.at("factor")));
  pack_lca1d(*self / div, out, oc);
}

void STRUCTLabelledControlArray1DPower(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  double exp  = std::holds_alternative<double>(pm.at("exponent"))
                    ? std::get<double>(pm.at("exponent"))
                    : static_cast<double>(std::get<int64_t>(pm.at("exponent")));
  pack_lca1d(*self ^ exp, out, oc);
}

// ── Reduction ─────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DMin(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->min()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DMax(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->max()}, out, 16, oc);
}

// ── Control metadata ──────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DPrincipleDimension(
    const FalconParamEntry *params, int32_t param_count, FalconResultSlot *out,
    int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(
      FunctionResult{static_cast<int64_t>(self->principle_dimension())}, out,
      16, oc);
}

void STRUCTLabelledControlArray1DAlignment(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_ia(self->alignment(), out, oc);
}

// ── Label metadata ────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DLabel(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_ac(self->label(), out, oc);
}

void STRUCTLabelledControlArray1DConnection(const FalconParamEntry *params,
                                             int32_t param_count,
                                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
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

void STRUCTLabelledControlArray1DInstrumentType(const FalconParamEntry *params,
                                                 int32_t param_count,
                                                 FalconResultSlot *out,
                                                 int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  auto inst = self->label()->instrument_type();
  pack_results(FunctionResult{inst}, out, 16, oc);
}

void STRUCTLabelledControlArray1DUnits(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto  self  = get_opaque<LabelledControlArray1D>(params, param_count, "this");
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

// ── 1D accessors ──────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DGetStart(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_start()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DGetEnd(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_end()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DIsDecreasing(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->is_decreasing()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DIsIncreasing(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->is_increasing()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DGetDistance(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_distance()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DGetMean(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_mean()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DGetSTD(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->get_std()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DReverse(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  self->reverse();
  out[0]     = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc        = 1;
}

void STRUCTLabelledControlArray1DGetClosestIndex(const FalconParamEntry *params,
                                                  int32_t param_count,
                                                  FalconResultSlot *out,
                                                  int32_t *oc) {
  auto   pm    = unpack_params(params, param_count);
  auto   self  = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  double value = std::holds_alternative<double>(pm.at("value"))
                     ? std::get<double>(pm.at("value"))
                     : static_cast<double>(std::get<int64_t>(pm.at("value")));
  size_t idx   = self->get_closest_index(value);
  pack_results(FunctionResult{static_cast<int64_t>(idx)}, out, 16, oc);
}

void STRUCTLabelledControlArray1DEvenDivisions(const FalconParamEntry *params,
                                                int32_t param_count,
                                                FalconResultSlot *out,
                                                int32_t *oc) {
  auto   pm        = unpack_params(params, param_count);
  auto   self      = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  size_t divisions = static_cast<size_t>(std::get<int64_t>(pm.at("divisions")));
  pack_farray_list(self->even_divisions(divisions), out, oc);
}

// ── Shape ─────────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DReshape(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto             pm      = unpack_params(params, param_count);
  auto             self    = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  auto             arr_val = std::get<std::shared_ptr<ArrayValue>>(pm.at("shape"));
  std::vector<size_t> shape;
  for (const auto &elem : arr_val->elements)
    shape.push_back(static_cast<size_t>(std::get<int64_t>(elem)));
  pack_lca1d(self->reshape(shape), out, oc);
}

void STRUCTLabelledControlArray1DFlip(const FalconParamEntry *params,
                                       int32_t param_count, FalconResultSlot *out,
                                       int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_lca1d(self->flip(axis), out, oc);
}

// ── Gradient ──────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DGradient(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  auto   self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  size_t axis = static_cast<size_t>(std::get<int64_t>(pm.at("axis")));
  pack_farray(self->gradient(axis), out, oc);
}

void STRUCTLabelledControlArray1DFullGradient(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_farray_list(self->gradient(), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DEqual(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  auto other = get_opaque<LabelledControlArray1D>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

void STRUCTLabelledControlArray1DNotEqual(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  auto other = get_opaque<LabelledControlArray1D>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

void STRUCTLabelledControlArray1DToJSON(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledControlArray1D>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

void STRUCTLabelledControlArray1DFromJSON(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto lca  = LabelledControlArray1D::from_json_string<LabelledControlArray1D>(json);
  pack_lca1d(lca, out, oc);
}

} // extern "C"
