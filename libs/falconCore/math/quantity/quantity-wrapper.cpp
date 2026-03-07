#include "falcon_core/math/Quantity.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Quantity   = falcon_core::math::Quantity;
using QuantitySP = std::shared_ptr<Quantity>;
using SymbolUnit   = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP = std::shared_ptr<SymbolUnit>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_quantity(QuantitySP q, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Quantity";
  out[0].value.opaque.ptr       = new QuantitySP(std::move(q));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<QuantitySP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(value: float, unit: SymbolUnit) -> (Quantity q)
void STRUCTQuantityNew(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto   pm   = unpack_params(params, param_count);
  double val  = std::get<double>(pm.at("value"));
  auto   unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  auto   q    = std::make_shared<Quantity>(val, unit);
  pack_quantity(std::move(q), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Value(this: Quantity) -> (float out)
void STRUCTQuantityValue(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto q = get_opaque<Quantity>(params, param_count, "this");
  pack_results(FunctionResult{q->value()}, out, 16, oc);
}

// Unit(this: Quantity) -> (SymbolUnit unit)
void STRUCTQuantityUnit(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto q    = get_opaque<Quantity>(params, param_count, "this");
  auto unit = q->unit();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr       = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: Quantity, other: Quantity) -> (bool equal)
void STRUCTQuantityEqual(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Quantity>(params, param_count, "this");
  auto other = get_opaque<Quantity>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Quantity, other: Quantity) -> (bool notequal)
void STRUCTQuantityNotEqual(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Quantity>(params, param_count, "this");
  auto other = get_opaque<Quantity>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── Unit conversion ───────────────────────────────────────────────────────────

// ConvertToUnit(this: Quantity, new_unit: SymbolUnit) -> (Quantity converted_state)
void STRUCTQuantityConvertToUnit(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto self     = get_opaque<Quantity>(params, param_count, "this");
  auto new_unit = get_opaque<SymbolUnit>(params, param_count, "new_unit");
  auto copy     = std::make_shared<Quantity>(*self);
  copy->convert_to(new_unit);
  pack_quantity(std::move(copy), out, oc);
}

// ── Arithmetic: Times ─────────────────────────────────────────────────────────

// Times(this: Quantity, factor: float|int|Quantity) -> (Quantity scaled_state)
void STRUCTQuantityTimes(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Quantity>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("factor"))) {
    double f = std::get<double>(pm.at("factor"));
    pack_quantity(*self * f, out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("factor"))) {
    int f = static_cast<int>(std::get<int64_t>(pm.at("factor")));
    pack_quantity(*self * f, out, oc);
  } else {
    auto other = get_opaque<Quantity>(params, param_count, "factor");
    pack_quantity(*self * other, out, oc);
  }
}

// ── Arithmetic: Divides ───────────────────────────────────────────────────────

// Divides(this: Quantity, divisor: float|int|Quantity) -> (Quantity scaled_state)
void STRUCTQuantityDivides(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Quantity>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("divisor"))) {
    double d = std::get<double>(pm.at("divisor"));
    pack_quantity(*self / d, out, oc);
  } else if (std::holds_alternative<int64_t>(pm.at("divisor"))) {
    int d = static_cast<int>(std::get<int64_t>(pm.at("divisor")));
    pack_quantity(*self / d, out, oc);
  } else {
    auto other = get_opaque<Quantity>(params, param_count, "divisor");
    pack_quantity(*self / other, out, oc);
  }
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: Quantity, exponent: float) -> (Quantity powered_state)
void STRUCTQuantityPower(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto   pm    = unpack_params(params, param_count);
  auto   self  = get_opaque<Quantity>(params, param_count, "this");
  double exp_d = std::get<double>(pm.at("exponent"));
  int    exp_i = static_cast<int>(exp_d);
  pack_quantity(*self ^ exp_i, out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

// Add(this: Quantity, other: Quantity) -> (Quantity sum_state)
void STRUCTQuantityAdd(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Quantity>(params, param_count, "this");
  auto other = get_opaque<Quantity>(params, param_count, "other");
  pack_quantity(*self + other, out, oc);
}

// ── Arithmetic: Subtract ──────────────────────────────────────────────────────

// Subtract(this: Quantity, other: Quantity) -> (Quantity difference_state)
void STRUCTQuantitySubtract(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Quantity>(params, param_count, "this");
  auto other = get_opaque<Quantity>(params, param_count, "other");
  pack_quantity(*self - other, out, oc);
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: Quantity) -> (Quantity negated_state)
void STRUCTQuantityNegate(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Quantity>(params, param_count, "this");
  pack_quantity(-*self, out, oc);
}

// Abs(this: Quantity) -> (Quantity absolute_state)
void STRUCTQuantityAbs(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Quantity>(params, param_count, "this");
  pack_quantity(self->abs(), out, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Quantity) -> (string json)
void STRUCTQuantityToJSON(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto q = get_opaque<Quantity>(params, param_count, "this");
  pack_results(FunctionResult{q->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Quantity dstate)
void STRUCTQuantityFromJSON(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto        q    = Quantity::from_json_string<Quantity>(json);
  pack_quantity(std::make_shared<Quantity>(*q), out, oc);
}

} // extern "C"
