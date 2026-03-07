#include "falcon_core/math/Point.hpp"
#include "falcon_core/math/Quantity.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include "falcon_core/generic/Map.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Point       = falcon_core::math::Point;
using PointSP     = std::shared_ptr<Point>;
using Quantity    = falcon_core::math::Quantity;
using QuantitySP  = std::shared_ptr<Quantity>;
using Connection  = falcon_core::physics::device_structures::Connection;
using ConnectionSP  = std::shared_ptr<Connection>;
using SymbolUnit  = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP  = std::shared_ptr<SymbolUnit>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_point(PointSP pt, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Point";
  out[0].value.opaque.ptr       = new PointSP(std::move(pt));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PointSP *>(p);
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

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// NewFromRaw(init: Map<Connection, float>, unit: SymbolUnit) -> (Point q)
void STRUCTPointNewFromRaw(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  auto pt   = std::make_shared<Point>(
      falcon_core::generic::Map<Connection, double>::create_empty(), unit);
  pack_point(std::move(pt), out, oc);
}

// NewFromQuantity(init: Map<Connection, Quantity>) -> (Point q)
void STRUCTPointNewFromQuantity(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto pt = std::make_shared<Point>();
  pack_point(std::move(pt), out, oc);
}

// ── Mutators ──────────────────────────────────────────────────────────────────

// InsertOrAssign(this: Point, key: Connection, value: Quantity) -> ()
void STRUCTPointInsertOrAssign(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto key  = get_opaque<Connection>(params, param_count, "key");
  auto val  = get_opaque<Quantity>(params, param_count, "value");
  self->insert_or_assign(key, val);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Insert(this: Point, key: Connection, value: Quantity) -> ()
void STRUCTPointInsert(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto key  = get_opaque<Connection>(params, param_count, "key");
  auto val  = get_opaque<Quantity>(params, param_count, "value");
  self->insert(key, val);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// SetUnit(this: Point, unit: SymbolUnit) -> ()
void STRUCTPointSetUnit(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  self->set_unit(unit);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Unit(this: Point) -> (SymbolUnit unit)
void STRUCTPointUnit(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
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

// Coordinates(this: Point) -> (Map<Connection, Quantity> map)
void STRUCTPointCoordinates(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Point>(params, param_count, "this");
  auto coord = self->coordinates();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Map";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(coord));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// Connections(this: Point) -> (Array<Connection> conns)
void STRUCTPointConnections(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self     = get_opaque<Point>(params, param_count, "this");
  auto conn_lst = self->connections();
  std::vector<RuntimeValue> elements;
  for (const auto &conn : conn_lst->items()) {
    auto inst           = std::make_shared<StructInstance>("Connection");
    inst->native_handle = std::static_pointer_cast<void>(conn);
    elements.push_back(inst);
  }
  auto arr_val = std::make_shared<ArrayValue>("Connection", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// ── Arithmetic ────────────────────────────────────────────────────────────────

// Times(this: Point, factor: float) -> (Point scaled_state)
void STRUCTPointTimes(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto   pm     = unpack_params(params, param_count);
  auto   self   = get_opaque<Point>(params, param_count, "this");
  double factor = std::get<double>(pm.at("factor"));
  pack_point(*self * factor, out, oc);
}

// Divides(this: Point, divisor: float) -> (Point scaled_state)
void STRUCTPointDivides(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto   pm      = unpack_params(params, param_count);
  auto   self    = get_opaque<Point>(params, param_count, "this");
  double divisor = std::get<double>(pm.at("divisor"));
  pack_point(*self / divisor, out, oc);
}

// Add(this: Point, other: Point) -> (Point sum_state)
void STRUCTPointAdd(const FalconParamEntry *params, int32_t param_count,
                    FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_point(*self + other, out, oc);
}

// Subtract(this: Point, other: Point) -> (Point difference_state)
void STRUCTPointSubtract(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_point(*self - other, out, oc);
}

// Negate(this: Point) -> (Point negated_state)
void STRUCTPointNegate(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  pack_point(-*self, out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: Point, other: Point) -> (bool equal)
void STRUCTPointEqual(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Point, other: Point) -> (bool notequal)
void STRUCTPointNotEqual(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Point) -> (string json)
void STRUCTPointToJSON(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Point dstate)
void STRUCTPointFromJSON(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto        pt   = Point::from_json_string<Point>(json);
  pack_point(std::make_shared<Point>(*pt), out, oc);
}

} // extern "C"
