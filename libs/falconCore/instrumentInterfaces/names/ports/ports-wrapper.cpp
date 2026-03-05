#include "falcon_core/instrument_interfaces/names/Ports.hpp"
#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Ports            = falcon_core::instrument_interfaces::names::Ports;
using PortsSP          = std::shared_ptr<Ports>;
using InstrumentPort   = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP = std::shared_ptr<InstrumentPort>;
using Connection       = falcon_core::physics::device_structures::Connection;
using ConnectionSP     = std::shared_ptr<Connection>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_opaque_ports(PortsSP ports, FalconResultSlot *out,
                               int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Ports";
  out[0].value.opaque.ptr       = new PortsSP(std::move(ports));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PortsSP *>(p);
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

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(ports: Array<InstrumentPort>) -> (Ports ports)
void STRUCTPortsNew(const FalconParamEntry *params, int32_t param_count,
                    FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  // The Array<InstrumentPort> arrives as an ArrayValue in the ParameterMap.
  // Each element is an opaque InstrumentPort shared_ptr.
  const auto &arr_val = std::get<ArrayValue>(pm.at("ports"));
  std::vector<InstrumentPortSP> vec;
  vec.reserve(arr_val.elements.size());
  for (const auto &elem : arr_val.elements) {
    vec.push_back(std::get<InstrumentPortSP>(elem));
  }
  auto ports_obj = std::make_shared<Ports>(vec);
  pack_opaque_ports(std::move(ports_obj), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Ports(this: Ports) -> (Array<InstrumentPort> ports)
void STRUCTPortsPorts(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto ports_obj  = get_opaque<Ports>(params, param_count, "this");
  const auto &vec = ports_obj->ports();
  auto *arr       = new FalconArray();
  arr->element_type = FALCON_TYPE_OPAQUE;
  arr->count        = static_cast<int32_t>(vec.size());
  arr->elements     = new FalconValue[arr->count];
  for (int32_t i = 0; i < arr->count; ++i) {
    arr->elements[i].opaque.type_name = "InstrumentPort";
    arr->elements[i].opaque.ptr       = new InstrumentPortSP(vec[i]);
    arr->elements[i].opaque.deleter   = [](void *p) {
      delete static_cast<InstrumentPortSP *>(p);
    };
  }
  out[0]             = {};
  out[0].tag         = FALCON_TYPE_ARRAY;
  out[0].value.array = arr;
  *oc = 1;
}

// GetDefaultNames(this: Ports) -> (Array<string> names)
void STRUCTPortsGetDefaultNames(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  auto names     = ports_obj->default_names();
  pack_results(FunctionResult{names}, out, 16, oc);
}

// GetPsuedoNames(this: Ports) -> (Array<Connection> connections)
void STRUCTPortsGetPsuedoNames(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto ports_obj    = get_opaque<Ports>(params, param_count, "this");
  const auto &conns = ports_obj->pseudo_names();
  auto *arr         = new FalconArray();
  arr->element_type = FALCON_TYPE_OPAQUE;
  arr->count        = static_cast<int32_t>(conns.size());
  arr->elements     = new FalconValue[arr->count];
  for (int32_t i = 0; i < arr->count; ++i) {
    arr->elements[i].opaque.type_name = "Connection";
    arr->elements[i].opaque.ptr       = new ConnectionSP(conns[i]);
    arr->elements[i].opaque.deleter   = [](void *p) {
      delete static_cast<ConnectionSP *>(p);
    };
  }
  out[0]             = {};
  out[0].tag         = FALCON_TYPE_ARRAY;
  out[0].value.array = arr;
  *oc = 1;
}

// IsKnobs(this: Ports) -> (Array<bool> is_knobs)
void STRUCTPortsIsKnobs(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  auto flags     = ports_obj->is_knobs();
  pack_results(FunctionResult{flags}, out, 16, oc);
}

// IsMeters(this: Ports) -> (Array<bool> is_meters)
void STRUCTPortsIsMeters(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  auto flags     = ports_obj->is_meters();
  pack_results(FunctionResult{flags}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: Ports, other: Ports) -> (bool equal)
void STRUCTPortsEqual(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Ports>(params, param_count, "this");
  auto other = get_opaque<Ports>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Ports, other: Ports) -> (bool notequal)
void STRUCTPortsNotEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Ports>(params, param_count, "this");
  auto other = get_opaque<Ports>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Ports) -> (string json)
void STRUCTPortsToJSON(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{ports_obj->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Ports ports)
void STRUCTPortsFromJSON(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto p    = Ports::from_json_string<Ports>(json);
  pack_opaque_ports(std::make_shared<Ports>(*p), out, oc);
}

} // extern "C"
