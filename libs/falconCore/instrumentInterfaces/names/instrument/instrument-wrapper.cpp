#include "falcon_core/instrument_interfaces/names/Instrument.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Instrument   = falcon_core::instrument_interfaces::names::Instrument;
using InstrumentSP = std::shared_ptr<Instrument>;

// ── helper ────────────────────────────────────────────────────────────────────

static void pack_opaque_instrument(InstrumentSP inst, FalconResultSlot *out,
                                   int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Instrument";
  out[0].value.opaque.ptr       = new InstrumentSP(std::move(inst));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<InstrumentSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Static string factories ───────────────────────────────────────────────────

// DCVoltageSource() -> (string type)
void STRUCTInstrumentDCVoltageSource(const FalconParamEntry *, int32_t,
                                      FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{std::string(Instrument::DCVoltageSource())},
               out, 16, oc);
}

// Amnmeter() -> (string type)
void STRUCTInstrumentAmnmeter(const FalconParamEntry *, int32_t,
                               FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{std::string(Instrument::Ammeter())}, out, 16, oc);
}

// Magnet() -> (string type)
void STRUCTInstrumentMagnet(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{std::string(Instrument::Magnet())}, out, 16, oc);
}

// VoltageSource() -> (string type)
void STRUCTInstrumentVoltageSource(const FalconParamEntry *, int32_t,
                                    FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{std::string(Instrument::VoltageSource())},
               out, 16, oc);
}

// Clock() -> (string type)
void STRUCTInstrumentClock(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{std::string(Instrument::Clock())}, out, 16, oc);
}

} // extern "C"
