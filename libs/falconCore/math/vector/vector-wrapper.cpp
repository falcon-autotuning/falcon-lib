#include "falcon_core/math/Vector.hpp"
#include "falcon_core/math/Point.hpp"
#include "falcon_core/communications/voltage_states/DeviceVoltageStates.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Vector    = falcon_core::math::Vector;
using VectorSP  = std::shared_ptr<Vector>;
using Point     = falcon_core::math::Point;
using PointSP   = std::shared_ptr<Point>;
using DeviceVoltageStates   = falcon_core::communications::voltage_states::DeviceVoltageStates;
using DeviceVoltageStatesSP = std::shared_ptr<DeviceVoltageStates>;
using Connection    = falcon_core::physics::device_structures::Connection;
using ConnectionSP  = std::shared_ptr<Connection>;
using SymbolUnit    = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP  = std::shared_ptr<SymbolUnit>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_vector(VectorSP v, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Vector";
  out[0].value.opaque.ptr       = new VectorSP(std::move(v));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<VectorSP *>(p);
  };
  *oc = 1;
}

static void pack_array_result(std::shared_ptr<ArrayValue> arr,
                               FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

static RuntimeValue wrap_conn_as_struct(ConnectionSP conn) {
  auto inst           = std::make_shared<StructInstance>("Connection");
  inst->native_handle = std::static_pointer_cast<void>(conn);
  return inst;
}

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// NewFromStart(start: Point) -> (Vector v)
void STRUCTVectorNewFromStart(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto start = get_opaque<Point>(params, param_count, "start");
  auto v     = std::make_shared<Vector>(start);
  pack_vector(std::move(v), out, oc);
}

// New(start: Point, end: Point) -> (Vector v)
void STRUCTVectorNew(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto start = get_opaque<Point>(params, param_count, "start");
  auto end   = get_opaque<Point>(params, param_count, "end");
  auto v     = std::make_shared<Vector>(start, end);
  pack_vector(std::move(v), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// EndPoint(this: Vector) -> (Point p)
void STRUCTVectorEndPoint(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  auto pt   = self->endPoint();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Point";
  out[0].value.opaque.ptr       = new PointSP(pt);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PointSP *>(p);
  };
  *oc = 1;
}

// StartPoint(this: Vector) -> (Point p)
void STRUCTVectorStartPoint(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  auto pt   = self->startPoint();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Point";
  out[0].value.opaque.ptr       = new PointSP(pt);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PointSP *>(p);
  };
  *oc = 1;
}

// Connections(this: Vector) -> (Array<Connection> conns)
void STRUCTVectorConnections(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Vector>(params, param_count, "this");
  auto conns = self->connections();
  std::vector<RuntimeValue> elements;
  for (const auto &conn : conns->items()) {
    elements.push_back(wrap_conn_as_struct(conn));
  }
  auto arr_val = std::make_shared<ArrayValue>("Connection", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// Unit(this: Vector) -> (SymbolUnit unit)
void STRUCTVectorUnit(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  auto unit = self->unit();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr       = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// PrincipleConnection(this: Vector) -> (Connection conn)
void STRUCTVectorPrincipleConnection(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  auto conn = self->principle_connection();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr       = new ConnectionSP(conn);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

// Magnitude(this: Vector) -> (float m)
void STRUCTVectorMagnitude(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  pack_results(FunctionResult{self->magnitude()}, out, 16, oc);
}

// ── Arithmetic ────────────────────────────────────────────────────────────────

// Times(this: Vector, factor: float|int) -> (Vector scaled_state)
void STRUCTVectorTimes(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Vector>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("factor")))
    pack_vector(*self * std::get<double>(pm.at("factor")), out, oc);
  else
    pack_vector(*self * static_cast<int>(std::get<int64_t>(pm.at("factor"))),
                out, oc);
}

// Divides(this: Vector, divisor: float|int) -> (Vector scaled_state)
void STRUCTVectorDivides(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Vector>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("divisor")))
    pack_vector(*self / std::get<double>(pm.at("divisor")), out, oc);
  else
    pack_vector(*self / static_cast<int>(std::get<int64_t>(pm.at("divisor"))),
                out, oc);
}

// Add(this: Vector, other: Vector) -> (Vector sum_state)
void STRUCTVectorAdd(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Vector>(params, param_count, "this");
  auto other = get_opaque<Vector>(params, param_count, "other");
  pack_vector(*self + other, out, oc);
}

// Subtract(this: Vector, other: Vector) -> (Vector diff)
void STRUCTVectorSubtract(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Vector>(params, param_count, "this");
  auto other = get_opaque<Vector>(params, param_count, "other");
  pack_vector(*self - other, out, oc);
}

// Negate(this: Vector) -> (Vector negated_state)
void STRUCTVectorNegate(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  pack_vector(-*self, out, oc);
}

// ── Spatial operations ────────────────────────────────────────────────────────

// UpdateStartFromStates(this: Vector, states: DeviceVoltageStates) -> (Vector v)
void STRUCTVectorUpdateStartFromStates(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self   = get_opaque<Vector>(params, param_count, "this");
  auto states = get_opaque<DeviceVoltageStates>(params, param_count, "states");
  pack_vector(self->update_start_from_states(states), out, oc);
}

// Translate(this: Vector, point: Point) -> (Vector v)
void STRUCTVectorTranslate(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Vector>(params, param_count, "this");
  auto point = get_opaque<Point>(params, param_count, "point");
  pack_vector(self->translate(point), out, oc);
}

// TranslateToOrigin(this: Vector) -> (Vector v)
void STRUCTVectorTranslateToOrigin(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  pack_vector(self->translate_to_origin(), out, oc);
}

// Extend(this: Vector, extension: float|int) -> (Vector v)
void STRUCTVectorExtend(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Vector>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("extension")))
    pack_vector(self->extend(std::get<double>(pm.at("extension"))), out, oc);
  else
    pack_vector(
        self->extend(static_cast<int>(std::get<int64_t>(pm.at("extension")))),
        out, oc);
}

// Shrink(this: Vector, extension: float|int) -> (Vector v)
void STRUCTVectorShrink(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Vector>(params, param_count, "this");
  if (std::holds_alternative<double>(pm.at("extension")))
    pack_vector(self->shrink(std::get<double>(pm.at("extension"))), out, oc);
  else
    pack_vector(
        self->shrink(static_cast<int>(std::get<int64_t>(pm.at("extension")))),
        out, oc);
}

// UnitVector(this: Vector) -> (Vector v)
void STRUCTVectorUnitVector(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  pack_vector(self->unit_vector(), out, oc);
}

// Normalize(this: Vector) -> (Vector v)
void STRUCTVectorNormalize(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  pack_vector(self->normalize(), out, oc);
}

// Project(this: Vector, other: Vector) -> (Vector v)
void STRUCTVectorProject(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Vector>(params, param_count, "this");
  auto other = get_opaque<Vector>(params, param_count, "other");
  pack_vector(self->project(other), out, oc);
}

// UpdateUnit(this: Vector, unit: SymbolUnit) -> ()
void STRUCTVectorUpdateUnit(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  self->update_unit(unit);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: Vector, other: Vector) -> (bool equal)
void STRUCTVectorEqual(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Vector>(params, param_count, "this");
  auto other = get_opaque<Vector>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Vector, other: Vector) -> (bool notequal)
void STRUCTVectorNotEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Vector>(params, param_count, "this");
  auto other = get_opaque<Vector>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Vector) -> (string json)
void STRUCTVectorToJSON(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Vector>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Vector dstate)
void STRUCTVectorFromJSON(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto        v    = Vector::from_json_string<Vector>(json);
  pack_vector(std::make_shared<Vector>(*v), out, oc);
}

} // extern "C"
