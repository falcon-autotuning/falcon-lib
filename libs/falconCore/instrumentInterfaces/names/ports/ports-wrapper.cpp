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

// Wrap an InstrumentPortSP as a StructInstance with native_handle — this is
// how the engine represents opaque FFI objects inside an ArrayValue element.
static RuntimeValue wrap_port_as_struct(InstrumentPortSP port) {
  auto inst = std::make_shared<StructInstance>("InstrumentPort");
  inst->native_handle = std::static_pointer_cast<void>(port);
  return inst;
}

// Wrap a ConnectionSP the same way.
static RuntimeValue wrap_conn_as_struct(ConnectionSP conn) {
  auto inst = std::make_shared<StructInstance>("Connection");
  inst->native_handle = std::static_pointer_cast<void>(conn);
  return inst;
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(ports: Array<InstrumentPort>) -> (Ports ports)
void STRUCTPortsNew(const FalconParamEntry *params, int32_t param_count,
                    FalconResultSlot *out, int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  std::shared_ptr<ArrayValue> arr_val = get_opaque<ArrayValue>(params, param_count, "ports");
  std::vector<InstrumentPortSP> vec;
  vec.reserve(arr_val->elements.size());
  auto elems = arr_val->elements;
  for (const auto &elem : elems) {
    std::shared_ptr<StructInstance> inst = std::get<std::shared_ptr<StructInstance>>(elem);
    std::shared_ptr<InstrumentPort> port = std::static_pointer_cast<InstrumentPort>(
        inst->native_handle.value());
    if (!port) throw std::runtime_error("InstrumentPort native_handle is null");
    vec.push_back(std::move(port));
  }
  pack_opaque_ports(std::make_shared<Ports>(vec), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Ports(this: Ports) -> (Array<InstrumentPort> ports)
void STRUCTPortsPorts(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  std::vector<RuntimeValue> elements;
  auto ports = *ports_obj->ports();
  for (const auto &port : ports) {
    elements.push_back(wrap_port_as_struct(port));
  }
  auto arr_val = std::make_shared<ArrayValue>("InstrumentPort",
                                              std::move(elements));
  pack_results(FunctionResult{arr_val}, out, 16, oc);
}

// GetDefaultNames(this: Ports) -> (Array<string> names)
//
// BUG FIX: keep the ListSP alive as a shared_ptr; never copy List<T> by value
// since its items() view points into internal storage that moves with the copy.
// Iterate via the shared_ptr dereference in-place, identical to GetPsuedoNames.
void STRUCTPortsGetDefaultNames(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  auto name_list = ports_obj->get_default_names(); // shared_ptr — NO copy
  std::vector<RuntimeValue> elements;
  for (const auto &name : *name_list) {            // dereference in range expr
    elements.push_back(name);
  }
  auto arr_val = std::make_shared<ArrayValue>("string", std::move(elements));
  pack_results(FunctionResult{arr_val}, out, 16, oc);
}

// GetPsuedoNames(this: Ports) -> (Array<Connection> connections)
void STRUCTPortsGetPsuedoNames(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  auto conn_list = ports_obj->get_pseudo_names();
  std::vector<RuntimeValue> elements;
  for (const auto &conn : *conn_list) {
    elements.push_back(wrap_conn_as_struct(conn));
  }
  auto arr_val = std::make_shared<ArrayValue>("Connection", std::move(elements));
  pack_results(FunctionResult{arr_val}, out, 16, oc);
}

// IsKnobs(this: Ports) -> (Array<bool> is_knobs)
void STRUCTPortsIsKnobs(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{ports_obj->is_knobs()}, out, 16, oc);
}

// IsMeters(this: Ports) -> (Array<bool> is_meters)
void STRUCTPortsIsMeters(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto ports_obj = get_opaque<Ports>(params, param_count, "this");
  pack_results(FunctionResult{ports_obj->is_meters()}, out, 16, oc);
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
