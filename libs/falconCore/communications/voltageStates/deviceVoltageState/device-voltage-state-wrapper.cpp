#include "falcon_core/communications/voltage_states/DeviceVoltageState.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using DeviceVoltageState   = falcon_core::communications::voltage_states::DeviceVoltageState;
using DeviceVoltageStateSP = std::shared_ptr<DeviceVoltageState>;
using Connection           = falcon_core::physics::device_structures::Connection;
using ConnectionSP         = std::shared_ptr<Connection>;
using SymbolUnit           = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP         = std::shared_ptr<SymbolUnit>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_dvs(DeviceVoltageStateSP dvs, FalconResultSlot *out,
                     int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceVoltageState";
  out[0].value.opaque.ptr       = new DeviceVoltageStateSP(std::move(dvs));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DeviceVoltageStateSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(conn: Connection, voltage: float, unit: SymbolUnit) -> (DeviceVoltageState state)
void STRUCTDeviceVoltageStateNew(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  auto conn    = get_opaque<Connection>(params, param_count, "conn");
  double volt  = std::get<double>(pm.at("voltage"));
  auto unit    = get_opaque<SymbolUnit>(params, param_count, "unit");
  auto dvs     = std::make_shared<DeviceVoltageState>(conn, volt, unit);
  pack_dvs(std::move(dvs), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Connection(this: DeviceVoltageState) -> (Connection conn)
void STRUCTDeviceVoltageStateConnection(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto dvs  = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto conn = dvs->connection();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr       = new ConnectionSP(conn);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

// Voltage(this: DeviceVoltageState) -> (float voltage)
void STRUCTDeviceVoltageStateVoltage(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto dvs = get_opaque<DeviceVoltageState>(params, param_count, "this");
  pack_results(FunctionResult{dvs->voltage()}, out, 16, oc);
}

// Unit(this: DeviceVoltageState) -> (SymbolUnit unit)
void STRUCTDeviceVoltageStateUnit(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto dvs  = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto unit = dvs->unit();
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

// Equal(this: DeviceVoltageState, other: DeviceVoltageState) -> (bool equal)
void STRUCTDeviceVoltageStateEqual(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self  = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto other = get_opaque<DeviceVoltageState>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: DeviceVoltageState, other: DeviceVoltageState) -> (bool notequal)
void STRUCTDeviceVoltageStateNotEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto other = get_opaque<DeviceVoltageState>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── Unit conversion ───────────────────────────────────────────────────────────

// ConvertToUnit(this: DeviceVoltageState, new_unit: SymbolUnit) -> (DeviceVoltageState converted_state)
void STRUCTDeviceVoltageStateConvertToUnit(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out, int32_t *oc) {
  auto dvs      = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto new_unit = get_opaque<SymbolUnit>(params, param_count, "new_unit");
  auto copy     = std::make_shared<DeviceVoltageState>(*dvs);
  copy->convert_to(new_unit);
  pack_dvs(std::move(copy), out, oc);
}

// ── Arithmetic: Times ─────────────────────────────────────────────────────────

void STRUCTDeviceVoltageStateTimes(const FalconParamEntry *params,
                                   int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<DeviceVoltageState>(params, param_count, "this");
  std::shared_ptr<DeviceVoltageState> result;
  if (std::holds_alternative<double>(pm.at("factor"))) {
    double fac = std::get<double>(pm.at("factor"));
    auto scaled = *self * fac;
    result = std::make_shared<DeviceVoltageState>(self->connection(), scaled->value(), self->unit());
  } else if (std::holds_alternative<int64_t>(pm.at("factor"))) {
    int fac = static_cast<int>(std::get<int64_t>(pm.at("factor")));
    auto scaled = *self * fac;
    result = std::make_shared<DeviceVoltageState>(self->connection(), scaled->value(), self->unit());
  } else {
    throw std::runtime_error("factor must be int or float");
  }
  pack_dvs(result, out, oc);
}

// Times(this: DVS, factor: Quantity) -> (DeviceVoltageState scaled_state)
void STRUCTDeviceVoltageStateTimesQuantity(const FalconParamEntry *params,
                                            int32_t param_count,
                                            FalconResultSlot *out, int32_t *oc) {
  auto self   = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto other  = get_opaque<DeviceVoltageState>(params, param_count, "factor");
  auto result = *self * std::static_pointer_cast<falcon_core::math::Quantity>(other);
  pack_dvs(std::make_shared<DeviceVoltageState>(self->connection(),
                                                result->value(), self->unit()),
           out, oc);
}

// ── Arithmetic: Divides ───────────────────────────────────────────────────────

void STRUCTDeviceVoltageStateDivides(const FalconParamEntry *params,
                                     int32_t param_count,
                                     FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<DeviceVoltageState>(params, param_count, "this");
  std::shared_ptr<DeviceVoltageState> result;
  if (std::holds_alternative<double>(pm.at("divisor"))) {
    double div = std::get<double>(pm.at("divisor"));
    auto scaled = *self / div;
    result = std::make_shared<DeviceVoltageState>(self->connection(), scaled->value(), self->unit());
  } else if (std::holds_alternative<int64_t>(pm.at("divisor"))) {
    int div = static_cast<int>(std::get<int64_t>(pm.at("divisor")));
    auto scaled = *self / div;
    result = std::make_shared<DeviceVoltageState>(self->connection(), scaled->value(), self->unit());
  } else {
    throw std::runtime_error("divisor must be int or float");
  }
  pack_dvs(result, out, oc);
}

// Divides(this: DVS, divisor: Quantity) -> (DeviceVoltageState scaled_state)
void STRUCTDeviceVoltageStateDividesQuantity(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto self   = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto other  = get_opaque<DeviceVoltageState>(params, param_count, "divisor");
  auto result = *self / std::static_pointer_cast<falcon_core::math::Quantity>(other);
  pack_dvs(std::make_shared<DeviceVoltageState>(self->connection(),
                                                result->value(), self->unit()),
           out, oc);
}

// ── Arithmetic: Power ─────────────────────────────────────────────────────────

// Power(this: DVS, exponent: float) -> (DeviceVoltageState powered_state)
void STRUCTDeviceVoltageStatePower(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto pm     = unpack_params(params, param_count);
  auto self   = get_opaque<DeviceVoltageState>(params, param_count, "this");
  int64_t exp = std::get<int64_t>(pm.at("exponent"));
  auto result = *self ^ static_cast<int>(exp);
  pack_dvs(std::make_shared<DeviceVoltageState>(self->connection(),
                                                result->value(), self->unit()),
           out, oc);
}

// ── Arithmetic: Add ───────────────────────────────────────────────────────────

void STRUCTDeviceVoltageStateAdd(const FalconParamEntry *params,
                                 int32_t param_count,
                                 FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<DeviceVoltageState>(params, param_count, "this");
  std::shared_ptr<DeviceVoltageState> result;
  if (std::holds_alternative<double>(pm.at("other"))) {
    double val = std::get<double>(pm.at("other"));
    auto sum = *self + val;
    result = std::make_shared<DeviceVoltageState>(self->connection(), sum->value(), self->unit());
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    int val = static_cast<int>(std::get<int64_t>(pm.at("other")));
    auto sum = *self + val;
    result = std::make_shared<DeviceVoltageState>(self->connection(), sum->value(), self->unit());
  } else if (auto other_dvs = get_opaque<DeviceVoltageState>(params, param_count, "other")) {
    auto sum = *self + std::static_pointer_cast<falcon_core::math::Quantity>(other_dvs);
    result = std::make_shared<DeviceVoltageState>(self->connection(), sum->value(), self->unit());
  } else {
    throw std::runtime_error("other must be int, float, or DeviceVoltageState");
  }
  pack_dvs(result, out, oc);
}

// ── Arithmetic: Subtract ──────────────────────────────────────────────────────

void STRUCTDeviceVoltageStateSubtract(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<DeviceVoltageState>(params, param_count, "this");
  std::shared_ptr<DeviceVoltageState> result;
  if (std::holds_alternative<double>(pm.at("other"))) {
    double val = std::get<double>(pm.at("other"));
    auto diff = *self - val;
    result = std::make_shared<DeviceVoltageState>(self->connection(), diff->value(), self->unit());
  } else if (std::holds_alternative<int64_t>(pm.at("other"))) {
    int val = static_cast<int>(std::get<int64_t>(pm.at("other")));
    auto diff = *self - val;
    result = std::make_shared<DeviceVoltageState>(self->connection(), diff->value(), self->unit());
  } else if (auto other_dvs = get_opaque<DeviceVoltageState>(params, param_count, "other")) {
    auto diff = *self - std::static_pointer_cast<falcon_core::math::Quantity>(other_dvs);
    result = std::make_shared<DeviceVoltageState>(self->connection(), diff->value(), self->unit());
  } else {
    throw std::runtime_error("other must be int, float, or DeviceVoltageState");
  }
  pack_dvs(result, out, oc);
}

// ── Unary ─────────────────────────────────────────────────────────────────────

// Negate(this: DVS) -> (DeviceVoltageState negated_state)
void STRUCTDeviceVoltageStateNegate(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self   = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto result = -*self;
  pack_dvs(std::make_shared<DeviceVoltageState>(self->connection(),
                                                result->value(), self->unit()),
           out, oc);
}

// Abs(this: DVS) -> (DeviceVoltageState absolute_state)
void STRUCTDeviceVoltageStateAbs(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self   = get_opaque<DeviceVoltageState>(params, param_count, "this");
  auto result = self->abs();
  pack_dvs(std::make_shared<DeviceVoltageState>(self->connection(),
                                                result->value(), self->unit()),
           out, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: DVS) -> (string json)
void STRUCTDeviceVoltageStateToJSON(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto dvs = get_opaque<DeviceVoltageState>(params, param_count, "this");
  pack_results(FunctionResult{dvs->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (DeviceVoltageState state)
void STRUCTDeviceVoltageStateFromJSON(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto dvs  = DeviceVoltageState::from_json_string<DeviceVoltageState>(json);
  pack_dvs(std::make_shared<DeviceVoltageState>(*dvs), out, oc);
}

} // extern "C"
