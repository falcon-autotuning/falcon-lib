#include "falcon_core/autotuner_interfaces/contexts/AcquisitionContext.hpp"
#include "falcon_core/autotuner_interfaces/contexts/MeasurementContext.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using AcquisitionContext   = falcon_core::autotuner_interfaces::contexts::AcquisitionContext;
using AcquisitionContextSP = std::shared_ptr<AcquisitionContext>;
using MeasurementContext   = falcon_core::autotuner_interfaces::contexts::MeasurementContext;
using MeasurementContextSP = std::shared_ptr<MeasurementContext>;
using BaseContext           = falcon_core::autotuner_interfaces::contexts::BaseContext;
using BaseContextSP        = std::shared_ptr<BaseContext>;
using Connection           = falcon_core::physics::device_structures::Connection;
using ConnectionSP         = std::shared_ptr<Connection>;
using SymbolUnit           = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP         = std::shared_ptr<SymbolUnit>;
using Instrument           = falcon_core::instrument_interfaces::names::Instrument;

// ── helpers ───────────────────────────────────────────────────────────────────

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

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// New(conn: Connection, inst: Instrument, unit: SymbolUnit) -> (AcquisitionContext context)
void STRUCTAcquisitionContextNew(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  auto inst = std::get<std::string>(pm.at("inst"));
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  auto ac   = std::make_shared<AcquisitionContext>(conn, inst, unit);
  pack_ac(std::move(ac), out, oc);
}

// NewFromContext(context: MeasurementContext, unit: SymbolUnit) -> (AcquisitionContext context)
void STRUCTAcquisitionContextNewFromContext(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out, int32_t *oc) {
  auto mc   = get_opaque<MeasurementContext>(params, param_count, "context");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  auto ac   = AcquisitionContext::from_context(
      std::static_pointer_cast<BaseContext>(mc), unit);
  pack_ac(std::move(ac), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Connection(this: AcquisitionContext) -> (Connection conn)
void STRUCTAcquisitionContextConnection(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto ac   = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto conn = ac->connection();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr       = new ConnectionSP(conn);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

// Instrument(this: AcquisitionContext) -> (Instrument inst)
void STRUCTAcquisitionContextInstrument(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto ac = get_opaque<AcquisitionContext>(params, param_count, "this");
  pack_results(FunctionResult{std::string(ac->instrument_type())}, out, 16, oc);
}

// Units(this: AcquisitionContext) -> (SymbolUnit unit)
void STRUCTAcquisitionContextUnits(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto ac   = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto unit = ac->units();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr       = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// ── Division ──────────────────────────────────────────────────────────────────

// Division(this: AcquisitionContext, other: AcquisitionContext) -> (AcquisitionContext out)
void STRUCTAcquisitionContextDivision(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto other = get_opaque<AcquisitionContext>(params, param_count, "other");
  auto result = *self / other;
  pack_ac(std::const_pointer_cast<AcquisitionContext>(result), out, oc);
}

// Division(this: AcquisitionContext, other: SymbolUnit) -> (AcquisitionContext out)
void STRUCTAcquisitionContextDivisionUnit(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto other = get_opaque<SymbolUnit>(params, param_count, "other");
  auto result = *self / other;
  pack_ac(std::const_pointer_cast<AcquisitionContext>(result), out, oc);
}

// ── Match predicates ──────────────────────────────────────────────────────────

// MatchConnection(this: AcquisitionContext, other: Connection) -> (bool match)
void STRUCTAcquisitionContextMatchConnection(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto ac    = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto other = get_opaque<Connection>(params, param_count, "other");
  pack_results(FunctionResult{ac->match_connection(other)}, out, 16, oc);
}

// MatchInstrumentType(this: AcquisitionContext, other: Instrument) -> (bool match)
void STRUCTAcquisitionContextMatchInstrumentType(const FalconParamEntry *params,
                                                  int32_t param_count,
                                                  FalconResultSlot *out,
                                                  int32_t *oc) {
  auto pm    = unpack_params(params, param_count);
  auto ac    = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto other = std::get<std::string>(pm.at("other"));
  pack_results(FunctionResult{ac->match_instrument_type(other)}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: AcquisitionContext, other: AcquisitionContext) -> (bool equal)
void STRUCTAcquisitionContextEqual(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self  = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto other = get_opaque<AcquisitionContext>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: AcquisitionContext, other: AcquisitionContext) -> (bool notequal)
void STRUCTAcquisitionContextNotEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<AcquisitionContext>(params, param_count, "this");
  auto other = get_opaque<AcquisitionContext>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: AcquisitionContext) -> (string json)
void STRUCTAcquisitionContextToJSON(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto ac = get_opaque<AcquisitionContext>(params, param_count, "this");
  pack_results(FunctionResult{ac->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (AcquisitionContext context)
void STRUCTAcquisitionContextFromJSON(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto ac   = AcquisitionContext::from_json_string<AcquisitionContext>(json);
  pack_ac(std::make_shared<AcquisitionContext>(*ac), out, oc);
}

} // extern "C"
