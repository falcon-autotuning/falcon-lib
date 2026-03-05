#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using InstrumentPort   = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP = std::shared_ptr<InstrumentPort>;
using Connection       = falcon_core::physics::device_structures::Connection;
using ConnectionSP     = std::shared_ptr<Connection>;
using SymbolUnit       = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP     = std::shared_ptr<SymbolUnit>;

// ── helpers ───────────────────────────────────────────────────────────────────

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

// ── Constructors ──────���───────────────────────────────────────────────────────

// NewKnob(default_name: string, conn: Connection, type: Instrument,
//         units: SymbolUnit, description: string) -> (InstrumentPort port)
void STRUCTInstrumentPortNewKnob(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm          = unpack_params(params, param_count);
  auto default_name = std::get<std::string>(pm.at("default_name"));
  auto conn        = get_opaque<Connection>(params, param_count, "conn");
  auto type_str    = std::get<std::string>(pm.at("type"));
  auto unit        = get_opaque<SymbolUnit>(params, param_count, "units");
  auto description = std::get<std::string>(pm.at("description"));
  auto port = InstrumentPort::Knob(default_name, conn, type_str, unit, description);
  pack_opaque_port(std::move(port), out, oc);
}

// NewMeter(default_name: string, conn: Connection, type: Instrument,
//          units: SymbolUnit, description: string) -> (InstrumentPort port)
void STRUCTInstrumentPortNewMeter(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto pm           = unpack_params(params, param_count);
  auto default_name = std::get<std::string>(pm.at("default_name"));
  auto conn         = get_opaque<Connection>(params, param_count, "conn");
  auto type_str     = std::get<std::string>(pm.at("type"));
  auto unit         = get_opaque<SymbolUnit>(params, param_count, "units");
  auto description  = std::get<std::string>(pm.at("description"));
  auto port = InstrumentPort::Meter(default_name, conn, type_str, unit, description);
  pack_opaque_port(std::move(port), out, oc);
}

// NewTimer() -> (InstrumentPort port)
void STRUCTInstrumentPortNewTimer(const FalconParamEntry *, int32_t,
                                   FalconResultSlot *out, int32_t *oc) {
  auto port = InstrumentPort::Timer();
  pack_opaque_port(std::move(port), out, oc);
}

// NewExecutionClock() -> (InstrumentPort port)
void STRUCTInstrumentPortNewExecutionClock(const FalconParamEntry *, int32_t,
                                            FalconResultSlot *out, int32_t *oc) {
  auto port = InstrumentPort::ExecutionClock();
  pack_opaque_port(std::move(port), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// DefaultName(this: InstrumentPort) -> (string default_name)
void STRUCTInstrumentPortDefaultName(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->default_name()}, out, 16, oc);
}

// PsuedoName(this: InstrumentPort) -> (Connection conn)
void STRUCTInstrumentPortPsuedoName(const FalconParamEntry *params,
                                     int32_t param_count,
                                     FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  auto conn = port->pseudo_name();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr       = new ConnectionSP(conn);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

// InstrumentType(this: InstrumentPort) -> (string type)
void STRUCTInstrumentPortInstrumentType(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{std::string(port->instrument_type())}, out, 16,
               oc);
}

// Units(this: InstrumentPort) -> (SymbolUnit units)
void STRUCTInstrumentPortUnits(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  auto unit = port->units();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr       = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// Description(this: InstrumentPort) -> (string description)
void STRUCTInstrumentPortDescription(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->description()}, out, 16, oc);
}

// InstrumentFacingName(this: InstrumentPort) -> (string facing_name)
void STRUCTInstrumentPortInstrumentFacingName(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->instrument_facing_name()}, out, 16, oc);
}

// IsKnob(this: InstrumentPort) -> (bool is_knob)
void STRUCTInstrumentPortIsKnob(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->is_knob()}, out, 16, oc);
}

// IsMeter(this: InstrumentPort) -> (bool is_meter)
void STRUCTInstrumentPortIsMeter(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->is_meter()}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: InstrumentPort, other: InstrumentPort) -> (bool equal)
void STRUCTInstrumentPortEqual(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self  = get_opaque<InstrumentPort>(params, param_count, "this");
  auto other = get_opaque<InstrumentPort>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: InstrumentPort, other: InstrumentPort) -> (bool notequal)
void STRUCTInstrumentPortNotEqual(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self  = get_opaque<InstrumentPort>(params, param_count, "this");
  auto other = get_opaque<InstrumentPort>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: InstrumentPort) -> (string json)
void STRUCTInstrumentPortToJSON(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "this");
  pack_results(FunctionResult{port->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (InstrumentPort port)
void STRUCTInstrumentPortFromJSON(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto port = InstrumentPort::from_json_string<InstrumentPort>(json);
  pack_opaque_port(std::make_shared<InstrumentPort>(*port), out, oc);
}

} // extern "C"
