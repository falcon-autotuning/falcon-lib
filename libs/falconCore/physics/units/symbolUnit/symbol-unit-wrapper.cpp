#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using SymbolUnit   = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP = std::shared_ptr<SymbolUnit>;

// ── helpers ──────────────────────────────────────────────────────────────────

static void pack_opaque_symbol_unit(SymbolUnitSP su, FalconResultSlot *out,
                                    int32_t *out_count) {
  out[0]                          = {};
  out[0].tag                      = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name   = "SymbolUnit";
  out[0].value.opaque.ptr         = new SymbolUnitSP(std::move(su));
  out[0].value.opaque.deleter     = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

void STRUCTSymbolUnitMeter(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Meter(), out, oc);
}
void STRUCTSymbolUnitKilogram(const FalconParamEntry *, int32_t,
                               FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Kilogram(), out, oc);
}
void STRUCTSymbolUnitSecond(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Second(), out, oc);
}
void STRUCTSymbolUnitAmpere(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Ampere(), out, oc);
}
void STRUCTSymbolUnitKelvin(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Kelvin(), out, oc);
}
void STRUCTSymbolUnitMole(const FalconParamEntry *, int32_t,
                           FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Mole(), out, oc);
}
void STRUCTSymbolUnitCandela(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Candela(), out, oc);
}
void STRUCTSymbolUnitHertz(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Hertz(), out, oc);
}
void STRUCTSymbolUnitNewton(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Newton(), out, oc);
}
void STRUCTSymbolUnitPascal(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Pascal(), out, oc);
}
void STRUCTSymbolUnitJoule(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Joule(), out, oc);
}
void STRUCTSymbolUnitWatt(const FalconParamEntry *, int32_t,
                           FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Watt(), out, oc);
}
void STRUCTSymbolUnitCoulomb(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Coulomb(), out, oc);
}
void STRUCTSymbolUnitVolt(const FalconParamEntry *, int32_t,
                           FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Volt(), out, oc);
}
void STRUCTSymbolUnitFarad(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Farad(), out, oc);
}
void STRUCTSymbolUnitOhm(const FalconParamEntry *, int32_t,
                          FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Ohm(), out, oc);
}
void STRUCTSymbolUnitSiemens(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Siemens(), out, oc);
}
void STRUCTSymbolUnitWeber(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Weber(), out, oc);
}
void STRUCTSymbolUnitTesla(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Tesla(), out, oc);
}
void STRUCTSymbolUnitHenry(const FalconParamEntry *, int32_t,
                            FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Henry(), out, oc);
}
void STRUCTSymbolUnitMinute(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Minute(), out, oc);
}
void STRUCTSymbolUnitHour(const FalconParamEntry *, int32_t,
                           FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Hour(), out, oc);
}
void STRUCTSymbolUnitElectronVolt(const FalconParamEntry *, int32_t,
                                   FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::ElectronVolt(), out, oc);
}
void STRUCTSymbolUnitCelsius(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Celsius(), out, oc);
}
void STRUCTSymbolUnitFahrenheit(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Fahrenheit(), out, oc);
}
void STRUCTSymbolUnitDimensionless(const FalconParamEntry *, int32_t,
                                    FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Dimensionless(), out, oc);
}
void STRUCTSymbolUnitPercent(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Percent(), out, oc);
}
void STRUCTSymbolUnitRadian(const FalconParamEntry *, int32_t,
                             FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::Radian(), out, oc);
}
void STRUCTSymbolUnitKilometer(const FalconParamEntry *, int32_t,
                                FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::KiloMeter(), out, oc);
}
void STRUCTSymbolUnitMiillimeter(const FalconParamEntry *, int32_t,
                                  FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MilliMeter(), out, oc);
}
void STRUCTSymbolUnitMiilivolt(const FalconParamEntry *, int32_t,
                                FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MilliVolt(), out, oc);
}
void STRUCTSymbolUnitKilovolt(const FalconParamEntry *, int32_t,
                               FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::KiloVolt(), out, oc);
}
void STRUCTSymbolUnitMilliampere(const FalconParamEntry *, int32_t,
                                  FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MilliAmpere(), out, oc);
}
void STRUCTSymbolUnitMicroampere(const FalconParamEntry *, int32_t,
                                  FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MicroAmpere(), out, oc);
}
void STRUCTSymbolUnitNanoampere(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::NanoAmpere(), out, oc);
}
void STRUCTSymbolUnitPicoampere(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::PicoAmpere(), out, oc);
}
void STRUCTSymbolUnitMillisecond(const FalconParamEntry *, int32_t,
                                  FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MilliSecond(), out, oc);
}
void STRUCTSymbolUnitMicrosecond(const FalconParamEntry *, int32_t,
                                  FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MicroSecond(), out, oc);
}
void STRUCTSymbolUnitNanosecond(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::NanoSecond(), out, oc);
}
void STRUCTSymbolUnitPicosecond(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::PicoSecond(), out, oc);
}
void STRUCTSymbolUnitMilliohm(const FalconParamEntry *, int32_t,
                               FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MilliOhm(), out, oc);
}
void STRUCTSymbolUnitKiloohm(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::KiloOhm(), out, oc);
}
void STRUCTSymbolUnitMegaohm(const FalconParamEntry *, int32_t,
                              FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MegaOhm(), out, oc);
}
void STRUCTSymbolUnitMillihertz(const FalconParamEntry *, int32_t,
                                 FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MilliHertz(), out, oc);
}
void STRUCTSymbolUnitKilohertz(const FalconParamEntry *, int32_t,
                                FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::KiloHertz(), out, oc);
}
void STRUCTSymbolUnitMegahertz(const FalconParamEntry *, int32_t,
                                FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MegaHertz(), out, oc);
}
void STRUCTSymbolUnitGigahertz(const FalconParamEntry *, int32_t,
                                FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::GigaHertz(), out, oc);
}
void STRUCTSymbolUnitMeterspersecond(const FalconParamEntry *, int32_t,
                                      FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MetersPerSecond(), out, oc);
}
void STRUCTSymbolUnitMeterspersecondSquared(const FalconParamEntry *, int32_t,
                                             FalconResultSlot *out,
                                             int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::MetersPerSecondSquared(), out, oc);
}
void STRUCTSymbolUnitNewtonmeter(const FalconParamEntry *, int32_t,
                                  FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::NewtonMeter(), out, oc);
}
void STRUCTSymbolUnitNewtonpermeter(const FalconParamEntry *, int32_t,
                                     FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::NewtonPerMeter(), out, oc);
}
void STRUCTSymbolUnitVoltspermeter(const FalconParamEntry *, int32_t,
                                    FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::VoltsPerMeter(), out, oc);
}
void STRUCTSymbolUnitVoltspersecond(const FalconParamEntry *, int32_t,
                                     FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::VoltsPerSecond(), out, oc);
}
void STRUCTSymbolUnitAmperespermeter(const FalconParamEntry *, int32_t,
                                      FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::AmperesPerMeter(), out, oc);
}
void STRUCTSymbolUnitVoltsperampere(const FalconParamEntry *, int32_t,
                                     FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::VoltsPerAmpere(), out, oc);
}
void STRUCTSymbolUnitWattspermeterkelvin(const FalconParamEntry *, int32_t,
                                          FalconResultSlot *out, int32_t *oc) {
  pack_opaque_symbol_unit(SymbolUnit::WattsPerMeterKelvin(), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Name(this: SymbolUnit) -> (string name)
void STRUCTSymbolUnitName(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto su = get_opaque<SymbolUnit>(params, param_count, "this");
  pack_results(FunctionResult{su->name()}, out, 16, oc);
}

// Symbol(this: SymbolUnit) -> (string symbol)
void STRUCTSymbolUnitSymbol(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto su = get_opaque<SymbolUnit>(params, param_count, "this");
  pack_results(FunctionResult{su->symbol()}, out, 16, oc);
}

// IsEqual(this: SymbolUnit, other: SymbolUnit) -> (bool equal)
void STRUCTSymbolUnitIsEqual(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto su    = get_opaque<SymbolUnit>(params, param_count, "this");
  auto other = get_opaque<SymbolUnit>(params, param_count, "other");
  pack_results(FunctionResult{*su == *other}, out, 16, oc);
}

// IsNotEqual(this: SymbolUnit, other: SymbolUnit) -> (bool notequal)
void STRUCTSymbolUnitIsNotEqual(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto su    = get_opaque<SymbolUnit>(params, param_count, "this");
  auto other = get_opaque<SymbolUnit>(params, param_count, "other");
  pack_results(FunctionResult{*su != *other}, out, 16, oc);
}

// ToJSON(this: SymbolUnit) -> (string json)
void STRUCTSymbolUnitToJSON(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto su = get_opaque<SymbolUnit>(params, param_count, "this");
  pack_results(FunctionResult{su->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (SymbolUnit unit)
void STRUCTSymbolUnitFromJSON(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  SymbolUnitSP su = SymbolUnit::from_json_string<SymbolUnit>(json);
  pack_opaque_symbol_unit(std::move(su), out, oc);
}

} // extern "C"
