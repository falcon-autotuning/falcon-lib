#include "falcon_core/autotuner_interfaces/contexts/MeasurementContext.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using MeasurementContext   = falcon_core::autotuner_interfaces::contexts::MeasurementContext;
using MeasurementContextSP = std::shared_ptr<MeasurementContext>;
using Connection           = falcon_core::physics::device_structures::Connection;
using ConnectionSP         = std::shared_ptr<Connection>;
using Instrument           = falcon_core::instrument_interfaces::names::Instrument;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_mc(MeasurementContextSP mc, FalconResultSlot *out,
                    int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "MeasurementContext";
  out[0].value.opaque.ptr       = new MeasurementContextSP(std::move(mc));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<MeasurementContextSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(conn: Connection, inst: Instrument) -> (MeasurementContext context)
void STRUCTMeasurementContextNew(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  auto conn    = get_opaque<Connection>(params, param_count, "conn");
  auto inst    = std::get<std::string>(pm.at("inst"));
  auto mc      = std::make_shared<MeasurementContext>(conn, inst);
  pack_mc(std::move(mc), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Connection(this: MeasurementContext) -> (Connection conn)
void STRUCTMeasurementContextConnection(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto mc   = get_opaque<MeasurementContext>(params, param_count, "this");
  auto conn = mc->connection();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr       = new ConnectionSP(conn);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

// Instrument(this: MeasurementContext) -> (Instrument inst)
void STRUCTMeasurementContextInstrument(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto mc = get_opaque<MeasurementContext>(params, param_count, "this");
  pack_results(FunctionResult{std::string(mc->instrument_type())}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: MeasurementContext, other: MeasurementContext) -> (bool equal)
void STRUCTMeasurementContextEqual(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self  = get_opaque<MeasurementContext>(params, param_count, "this");
  auto other = get_opaque<MeasurementContext>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: MeasurementContext, other: MeasurementContext) -> (bool notequal)
void STRUCTMeasurementContextNotEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasurementContext>(params, param_count, "this");
  auto other = get_opaque<MeasurementContext>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: MeasurementContext) -> (string json)
void STRUCTMeasurementContextToJSON(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto mc = get_opaque<MeasurementContext>(params, param_count, "this");
  pack_results(FunctionResult{mc->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (MeasurementContext context)
void STRUCTMeasurementContextFromJSON(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto mc   = MeasurementContext::from_json_string<MeasurementContext>(json);
  pack_mc(std::make_shared<MeasurementContext>(*mc), out, oc);
}

} // extern "C"
