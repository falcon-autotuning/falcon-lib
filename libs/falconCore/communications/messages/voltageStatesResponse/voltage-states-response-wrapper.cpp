#include "falcon_core/communications/messages/VoltageStatesResponse.hpp"
#include "falcon_core/communications/voltage_states/DeviceVoltageStates.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using VoltageStatesResponse   = falcon_core::communications::messages::VoltageStatesResponse;
using VoltageStatesResponseSP = std::shared_ptr<VoltageStatesResponse>;
using DeviceVoltageStates     = falcon_core::communications::voltage_states::DeviceVoltageStates;
using DeviceVoltageStatesSP   = std::shared_ptr<DeviceVoltageStates>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_vsr(VoltageStatesResponseSP vsr, FalconResultSlot *out,
                     int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "VoltageStatesResponse";
  out[0].value.opaque.ptr       = new VoltageStatesResponseSP(std::move(vsr));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<VoltageStatesResponseSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(message: string, states: DeviceVoltageStates) -> (VoltageStatesResponse response)
void STRUCTVoltageStatesResponseNew(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  auto message = std::get<std::string>(pm.at("message"));
  auto states  = get_opaque<DeviceVoltageStates>(params, param_count, "states");
  auto vsr     = std::make_shared<VoltageStatesResponse>(message, states);
  pack_vsr(std::move(vsr), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// States(this: VoltageStatesResponse) -> (DeviceVoltageStates states)
void STRUCTVoltageStatesResponseStates(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto vsr    = get_opaque<VoltageStatesResponse>(params, param_count, "this");
  auto states = vsr->states();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceVoltageStates";
  out[0].value.opaque.ptr       = new DeviceVoltageStatesSP(states);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DeviceVoltageStatesSP *>(p);
  };
  *oc = 1;
}

// Message(this: VoltageStatesResponse) -> (string message)
void STRUCTVoltageStatesResponseMessage(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto vsr = get_opaque<VoltageStatesResponse>(params, param_count, "this");
  pack_results(FunctionResult{vsr->message()}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: VoltageStatesResponse, other: VoltageStatesResponse) -> (bool equal)
void STRUCTVoltageStatesResponseEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<VoltageStatesResponse>(params, param_count, "this");
  auto other = get_opaque<VoltageStatesResponse>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: VoltageStatesResponse, other: VoltageStatesResponse) -> (bool notequal)
void STRUCTVoltageStatesResponseNotEqual(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<VoltageStatesResponse>(params, param_count, "this");
  auto other = get_opaque<VoltageStatesResponse>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: VoltageStatesResponse) -> (string json)
void STRUCTVoltageStatesResponseToJSON(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto vsr = get_opaque<VoltageStatesResponse>(params, param_count, "this");
  pack_results(FunctionResult{vsr->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (VoltageStatesResponse response)
void STRUCTVoltageStatesResponseFromJSON(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto vsr =
      VoltageStatesResponse::from_json_string<VoltageStatesResponse>(json);
  pack_vsr(std::make_shared<VoltageStatesResponse>(*vsr), out, oc);
}

} // extern "C"
