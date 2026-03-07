#include "falcon_core/instrument_interfaces/port_transforms/PortTransform.hpp"
#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/math/AnalyticFunction.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using PortTransform      = falcon_core::instrument_interfaces::port_transforms::PortTransform;
using PortTransformSP    = std::shared_ptr<PortTransform>;
using InstrumentPort     = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP   = std::shared_ptr<InstrumentPort>;
using AnalyticFunction   = falcon_core::math::AnalyticFunction;
using AnalyticFunctionSP = std::shared_ptr<AnalyticFunction>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_opaque_pt(PortTransformSP pt, FalconResultSlot *out,
                            int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "PortTransform";
  out[0].value.opaque.ptr       = new PortTransformSP(std::move(pt));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PortTransformSP *>(p);
  };
  *oc = 1;
}

static void pack_opaque_port(InstrumentPortSP port, FalconResultSlot *out,
                              int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "InstrumentPort";
  out[0].value.opaque.ptr       = new InstrumentPortSP(std::move(port));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<InstrumentPortSP *>(p);
  };
  *oc = 1;
}

static void pack_array_result(std::shared_ptr<ArrayValue> arr,
                               FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// New(port: InstrumentPort, func: AnalyticFunction) -> (PortTransform transform)
void STRUCTPortTransformNew(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "port");
  auto func = get_opaque<AnalyticFunction>(params, param_count, "func");
  auto pt   = std::make_shared<PortTransform>(port, func);
  pack_opaque_pt(std::move(pt), out, oc);
}

// ConstantTransform(port: InstrumentPort, value: float) -> (PortTransform transform)
void STRUCTPortTransformConstantTransform(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto pm    = unpack_params(params, param_count);
  auto port  = get_opaque<InstrumentPort>(params, param_count, "port");
  double val = std::get<double>(pm.at("value"));
  auto pt    = PortTransform::ConstantTransform(port, val);
  pack_opaque_pt(std::move(pt), out, oc);
}

// IdentityTranform(port: InstrumentPort) -> (PortTransform transform)
void STRUCTPortTransformIdentityTranform(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "port");
  auto pt   = PortTransform::IdentityTransform(port);
  pack_opaque_pt(std::move(pt), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Port(this: PortTransform) -> (InstrumentPort port)
void STRUCTPortTransformPort(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<PortTransform>(params, param_count, "this");
  auto port = self->port();
  pack_opaque_port(port, out, oc);
}

// Labels(this: PortTransform) -> (Array<string> variables)
void STRUCTPortTransformLabels(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self       = get_opaque<PortTransform>(params, param_count, "this");
  auto label_list = self->labels();
  std::vector<RuntimeValue> elements;
  for (const auto &lbl : *label_list) {
    elements.push_back(lbl);
  }
  auto arr_val = std::make_shared<ArrayValue>("string", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: PortTransform, other: PortTransform) -> (bool equal)
void STRUCTPortTransformEqual(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self  = get_opaque<PortTransform>(params, param_count, "this");
  auto other = get_opaque<PortTransform>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: PortTransform, other: PortTransform) -> (bool notequal)
void STRUCTPortTransformNotEqual(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self  = get_opaque<PortTransform>(params, param_count, "this");
  auto other = get_opaque<PortTransform>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: PortTransform) -> (string json)
void STRUCTPortTransformToJSON(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self = get_opaque<PortTransform>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (PortTransform transform)
void STRUCTPortTransformFromJSON(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto pt   = PortTransform::from_json_string<PortTransform>(json);
  pack_opaque_pt(std::make_shared<PortTransform>(*pt), out, oc);
}

} // extern "C"
