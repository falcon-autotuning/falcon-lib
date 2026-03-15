#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/device_structures/Connections.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Connections = falcon_core::physics::device_structures::Connections;
using ConnectionsSP = std::shared_ptr<Connections>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;

// ── helpers
// ───────────────────────────────────────────────────────────────────

static void pack_opaque_connections(ConnectionsSP conns, FalconResultSlot *out,
                                    int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connections";
  out[0].value.opaque.ptr = new ConnectionsSP(std::move(conns));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ConnectionsSP *>(p);
  };
  *oc = 1;
}

static void pack_opaque_connection(ConnectionSP conn, FalconResultSlot *out,
                                   int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Connection";
  out[0].value.opaque.ptr = new ConnectionSP(std::move(conn));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<ConnectionSP *>(p);
  };
  *oc = 1;
}

static RuntimeValue wrap_conn_as_struct(ConnectionSP conn) {
  auto inst = std::make_shared<StructInstance>("Connection");
  inst->native_handle = std::static_pointer_cast<void>(conn);
  return inst;
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

static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) {
      continue;
    }

    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(
          std::string("STRUCTConnectionsNew: parameter '") + key +
          "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    }

    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";

    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue") {
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    }

    throw std::runtime_error(std::string("STRUCTConnectionsNew: parameter '") +
                             key + "' has unexpected opaque type_name='" + tn +
                             "'");
  }
  throw std::runtime_error(std::string("STRUCTConnectionsNew: parameter '") +
                           key + "' not found");
}

extern "C" {

// ── Constructor
// ───────────────────────────────────────────────────────────────

// New(conns: Array<Connection>) -> (Connections c)
void STRUCTConnectionsNew(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "conns");

  std::vector<ConnectionSP> vec;
  vec.reserve(arr_val->elements.size());

  for (const auto &elem : arr_val->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem)) {
      throw std::runtime_error(
          "STRUCTConnectionsNew: array element is not a StructInstance");
    }
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value()) {
      throw std::runtime_error("STRUCTConnectionsNew: Connection "
                               "StructInstance has no native_handle");
    }
    vec.push_back(
        std::static_pointer_cast<Connection>(inst->native_handle.value()));
  }

  pack_opaque_connections(std::make_shared<Connections>(vec), out, oc);
}

// ── Type predicates
// ───────────────────────────────────────────────────────────

// IsGates(this: Connections) -> (bool IsGates)
void STRUCTConnectionsIsGates(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->is_gates()}, out, 16, oc);
}

// IsOhmics(this: Connections) -> (bool IsOhmics)
void STRUCTConnectionsIsOhmics(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->is_ohmics()}, out, 16, oc);
}

// IsDotGates(this: Connections) -> (bool IsDotGates)
void STRUCTConnectionsIsDotGates(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->is_dot_gates()}, out, 16, oc);
}

// IsPlungerGates(this: Connections) -> (bool IsPlungerGates)
void STRUCTConnectionsIsPlungerGates(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->is_plunger_gates()}, out, 16, oc);
}

// IsBarrierGates(this: Connections) -> (bool IsBarrierGates)
void STRUCTConnectionsIsBarrierGates(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->is_barrier_gates()}, out, 16, oc);
}

// IsReservoirGates(this: Connections) -> (bool IsReservoirGates)
void STRUCTConnectionsIsReservoirGates(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->is_reservoir_gates()}, out, 16, oc);
}

// IsScreeningGates(this: Connections) -> (bool IsScreeningGates)
void STRUCTConnectionsIsScreeningGates(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->is_screening_gates()}, out, 16, oc);
}

// ── Set operations
// ────────────────────────────────────────────────────────────

// Intersection(this: Connections, other: Connections) -> (Connections out)
void STRUCTConnectionsIntersection(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  auto other = get_opaque<Connections>(params, param_count, "other");
  auto result = self->intersection(other);
  pack_opaque_connections(result, out, oc);
}

// ── Equality
// ──────────────────────────────────────────────────────────────────

// Equal(this: Connections, other: Connections) -> (bool equal)
void STRUCTConnectionsEqual(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  auto other = get_opaque<Connections>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Connections, other: Connections) -> (bool notequal)
void STRUCTConnectionsNotEqual(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  auto other = get_opaque<Connections>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON
// ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Connections) -> (string json)
void STRUCTConnectionsToJSON(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Connections conn)
void STRUCTConnectionsFromJSON(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto conns = Connections::from_json_string<Connections>(json);
  pack_opaque_connections(std::make_shared<Connections>(*conns), out, oc);
}

// ── List accessors
// ────────────────────────────────────────────────────────────

// Size(this: Connections) -> (int size)
void STRUCTConnectionsSize(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->size())}, out, 16, oc);
}

// IsEmpty(this: Connections) -> (bool empty)
void STRUCTConnectionsIsEmpty(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

// GetIndex(this: Connections, index: int) -> (T value)
void STRUCTConnectionsGetIndex(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Connections>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto conn = self->at(static_cast<size_t>(idx));
  pack_opaque_connection(conn, out, oc);
}

// PushBack(this: Connections, value: T) -> ()
void STRUCTConnectionsPushBack(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "value");
  self->push_back(conn);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Insert(this: Connections, index: int, value: T) -> ()
void STRUCTConnectionsInsert(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Connections>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto conn = get_opaque<Connection>(params, param_count, "value");
  std::vector<ConnectionSP> tmp{conn};
  auto it = self->begin();
  std::advance(it, static_cast<size_t>(idx));
  self->insert(it, tmp.begin(), tmp.end());
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Contains(this: Connections, value: T) -> (bool found)
void STRUCTConnectionsContains(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "value");
  pack_results(FunctionResult{self->contains(conn)}, out, 16, oc);
}

// Index(this: Connections, value: T) -> (int index)
void STRUCTConnectionsIndex(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "value");
  pack_results(FunctionResult{static_cast<int64_t>(self->index(conn))}, out, 16,
               oc);
}

// Items(this: Connections) -> (Array<Connection> items)
void STRUCTConnectionsItems(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  std::vector<RuntimeValue> elements;
  for (const auto &conn : self->items()) {
    elements.push_back(wrap_conn_as_struct(conn));
  }
  auto arr_val =
      std::make_shared<ArrayValue>("Connection", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// Clear(this: Connections) -> ()
void STRUCTConnectionsClear(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Connections>(params, param_count, "this");
  self->clear();
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Erase(this: Connections, index: int) -> ()
void STRUCTConnectionsErase(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<Connections>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  self->erase_at(static_cast<size_t>(idx));
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

} // extern "C"
