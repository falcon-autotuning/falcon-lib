#include "falcon_core/generic/Map.hpp"
#include "falcon_core/math/Point.hpp"
#include "falcon_core/math/Quantity.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Point = falcon_core::math::Point;
using PointSP = std::shared_ptr<Point>;
using Quantity = falcon_core::math::Quantity;
using QuantitySP = std::shared_ptr<Quantity>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;
using SymbolUnit = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP = std::shared_ptr<SymbolUnit>;

// ── helpers
// ───────────────────────────────────────────────────────────────────

static void pack_point(PointSP pt, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Point";
  out[0].value.opaque.ptr = new PointSP(std::move(pt));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<PointSP *>(p);
  };
  *oc = 1;
}

static void pack_array_result(std::shared_ptr<ArrayValue> arr,
                              FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}
// ── Helper to extract a DSL Map StructInstance from parameters
static std::shared_ptr<StructInstance>
get_map_param(const FalconParamEntry *params, int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(params[i].key, key) != 0)
      continue;
    const FalconParamEntry &e = params[i];
    if (e.tag != FALCON_TYPE_OPAQUE)
      throw std::runtime_error(std::string("get_map_param: parameter '") + key +
                               "' is not OPAQUE");
    auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
    return std::static_pointer_cast<StructInstance>(sv);
  }
  throw std::runtime_error(std::string("get_map_param: parameter '") + key +
                           "' not found");
}

// ── Helper to extract ArrayValue from a Map field
static std::shared_ptr<ArrayValue>
get_struct_field_array(const std::shared_ptr<StructInstance> &inst,
                       const char *field_name) {
  if (!inst)
    throw std::runtime_error(
        std::string("get_struct_field_array: null StructInstance for field '") +
        field_name + "'");
  auto it = inst->fields->find(field_name);
  if (it == inst->fields->end())
    throw std::runtime_error(std::string("get_struct_field_array: field '") +
                             field_name + "' not found");
  const RuntimeValue &fv = it->second;
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(fv)) {
    auto farr_inst = std::get<std::shared_ptr<StructInstance>>(fv);
    if (farr_inst && farr_inst->native_handle.has_value()) {
      return std::static_pointer_cast<ArrayValue>(
          farr_inst->native_handle.value());
    }
  }
  throw std::runtime_error(std::string("get_struct_field_array: field '") +
                           field_name +
                           "' is not a StructInstance with native_handle");
}

extern "C" {

// ── Constructors
// ──────────────────────────────────────────────────────────────

// ── NewFromRaw(init: Map<Connection, float>, unit: SymbolUnit) -> (Point p)
void STRUCTPointNewFromRaw(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto map_inst = get_map_param(params, param_count, "init");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");

  // Extract keys and values arrays from the DSL Map
  auto keys_arr = get_struct_field_array(map_inst, "keys_");
  auto vals_arr = get_struct_field_array(map_inst, "values_");

  if (keys_arr->elements.size() != vals_arr->elements.size()) {
    throw std::runtime_error(
        "NewFromRaw: Map keys_ and values_ have different sizes");
  }

  // Build the falconCore Map from the DSL Map
  auto core_map =
      std::make_shared<falcon_core::generic::Map<Connection, double>>();

  for (size_t i = 0; i < keys_arr->elements.size(); ++i) {
    // Extract key (Connection) from StructInstance
    const RuntimeValue &kv = keys_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(kv)) {
      throw std::runtime_error("NewFromRaw: Map key is not a StructInstance");
    }
    auto k_inst = std::get<std::shared_ptr<StructInstance>>(kv);
    if (!k_inst || !k_inst->native_handle.has_value()) {
      throw std::runtime_error(
          "NewFromRaw: Map key StructInstance has no native_handle");
    }
    auto conn =
        std::static_pointer_cast<Connection>(k_inst->native_handle.value());

    // Extract value (float) from RuntimeValue
    const RuntimeValue &vv = vals_arr->elements[i];
    double val = std::get<double>(vv);

    core_map->insert(conn, val);
  }

  auto pt = std::make_shared<Point>(core_map, unit);
  pack_point(std::move(pt), out, oc);
}

// ── NewFromQuantity(init: Map<Connection, Quantity>) -> (Point p)
void STRUCTPointNewFromQuantity(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto map_inst = get_map_param(params, param_count, "init");

  // Extract keys and values arrays from the DSL Map
  auto keys_arr = get_struct_field_array(map_inst, "keys_");
  auto vals_arr = get_struct_field_array(map_inst, "values_");

  if (keys_arr->elements.size() != vals_arr->elements.size()) {
    throw std::runtime_error(
        "NewFromQuantity: Map keys_ and values_ have different sizes");
  }

  // Build the falconCore Map from the DSL Map
  auto core_map =
      std::make_shared<falcon_core::generic::Map<Connection, Quantity>>();

  for (size_t i = 0; i < keys_arr->elements.size(); ++i) {
    // Extract key (Connection) from StructInstance
    const RuntimeValue &kv = keys_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(kv)) {
      throw std::runtime_error(
          "NewFromQuantity: Map key is not a StructInstance");
    }
    auto k_inst = std::get<std::shared_ptr<StructInstance>>(kv);
    if (!k_inst || !k_inst->native_handle.has_value()) {
      throw std::runtime_error(
          "NewFromQuantity: Map key StructInstance has no native_handle");
    }
    auto conn =
        std::static_pointer_cast<Connection>(k_inst->native_handle.value());

    // Extract value (Quantity) from StructInstance
    const RuntimeValue &vv = vals_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(vv)) {
      throw std::runtime_error(
          "NewFromQuantity: Map value is not a StructInstance");
    }
    auto v_inst = std::get<std::shared_ptr<StructInstance>>(vv);
    if (!v_inst || !v_inst->native_handle.has_value()) {
      throw std::runtime_error(
          "NewFromQuantity: Map value StructInstance has no native_handle");
    }
    auto qty =
        std::static_pointer_cast<Quantity>(v_inst->native_handle.value());

    core_map->insert(conn, qty);
  }

  auto pt = std::make_shared<Point>(core_map);
  pack_point(std::move(pt), out, oc);
}

// ── Mutators
// ──────────────────────────────────────────────────────────────────

// InsertOrAssign(this: Point, key: Connection, value: Quantity) -> ()
void STRUCTPointInsertOrAssign(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto key = get_opaque<Connection>(params, param_count, "key");
  auto val = get_opaque<Quantity>(params, param_count, "value");
  self->insert_or_assign(key, val);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Insert(this: Point, key: Connection, value: Quantity) -> ()
void STRUCTPointInsert(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto key = get_opaque<Connection>(params, param_count, "key");
  auto val = get_opaque<Quantity>(params, param_count, "value");
  self->insert(key, val);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// SetUnit(this: Point, unit: SymbolUnit) -> ()
void STRUCTPointSetUnit(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  self->set_unit(unit);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── Accessors
// ─────────────────────────────────────────────────────────────────

// Unit(this: Point) -> (SymbolUnit unit)
void STRUCTPointUnit(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto unit = self->unit();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// Coordinates(this: Point) -> (Map<Connection, Quantity> map)
void STRUCTPointCoordinates(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto coord = self->coordinates();

  // Create ArrayValue for keys (Array<Connection>)
  auto keys_array = std::make_shared<ArrayValue>("Connection");
  keys_array->elements.reserve(coord->size());

  // Create ArrayValue for values (Array<Quantity>)
  auto values_array = std::make_shared<ArrayValue>("Quantity");
  values_array->elements.reserve(coord->size());

  // Populate keys and values arrays from the map
  for (const auto &entry : *coord->items()) {
    // Wrap Connection as StructInstance with native_handle
    auto key_inst = std::make_shared<StructInstance>("Connection");
    key_inst->native_handle = std::static_pointer_cast<void>(entry->first());
    keys_array->elements.push_back(key_inst);

    // Wrap Quantity as StructInstance with native_handle
    auto value_inst = std::make_shared<StructInstance>("Quantity");
    value_inst->native_handle = std::static_pointer_cast<void>(entry->second());
    values_array->elements.push_back(value_inst);
  }

  // Wrap each ArrayValue as a StructInstance with native_handle,
  // matching the Array<T> FFI binding convention
  auto keys_field = std::make_shared<StructInstance>("Array");
  keys_field->native_handle = std::static_pointer_cast<void>(keys_array);

  auto values_field = std::make_shared<StructInstance>("Array");
  values_field->native_handle = std::static_pointer_cast<void>(values_array);

  // Build the Map StructInstance (pure FAL struct, no native_handle)
  auto map_inst = std::make_shared<StructInstance>("Map");
  map_inst->fields->insert(std::make_pair("keys_", keys_field));
  map_inst->fields->insert(std::make_pair("values_", values_field));

  // Pack as StructInstance (plain FAL struct) — NOT as OPAQUE
  // This will be unpacked correctly as a FAL struct with native Array fields
  FunctionResult result;
  result.push_back(map_inst);
  pack_results(result, out, 1, oc);
}

// Connections(this: Point) -> (Array<Connection> conns)
void STRUCTPointConnections(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto conn_lst = self->connections();
  std::vector<RuntimeValue> elements;
  for (const auto &conn : conn_lst->items()) {
    auto inst = std::make_shared<StructInstance>("Connection");
    inst->native_handle = std::static_pointer_cast<void>(conn);
    elements.push_back(inst);
  }
  auto arr_val =
      std::make_shared<ArrayValue>("Connection", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// ── Arithmetic
// ────────────────────────────────────────────────────────────────

// Times(this: Point, factor: float) -> (Point scaled_state)
void STRUCTPointTimes(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Point>(params, param_count, "this");
  double factor = std::get<double>(pm.at("factor"));
  pack_point(*self * factor, out, oc);
}

// Divides(this: Point, divisor: float) -> (Point scaled_state)
void STRUCTPointDivides(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Point>(params, param_count, "this");
  double divisor = std::get<double>(pm.at("divisor"));
  pack_point(*self / divisor, out, oc);
}

// Add(this: Point, other: Point) -> (Point sum_state)
void STRUCTPointAdd(const FalconParamEntry *params, int32_t param_count,
                    FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_point(*self + other, out, oc);
}

// Subtract(this: Point, other: Point) -> (Point difference_state)
void STRUCTPointSubtract(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_point(*self - other, out, oc);
}

// Negate(this: Point) -> (Point negated_state)
void STRUCTPointNegate(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  pack_point(-*self, out, oc);
}

// ── Equality
// ──────────────────────────────────────────────────────────────────

// Equal(this: Point, other: Point) -> (bool equal)
void STRUCTPointEqual(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Point, other: Point) -> (bool notequal)
void STRUCTPointNotEqual(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  auto other = get_opaque<Point>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON
// ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Point) -> (string json)
void STRUCTPointToJSON(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Point>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Point dstate)
void STRUCTPointFromJSON(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto pt = Point::from_json_string<Point>(json);
  pack_point(std::move(pt), out, oc);
}

} // extern "C"
