/**
 * map-wrapper.cpp — FFI implementation for Map<K,V>.
 *
 * The backing store is a std::map<RuntimeValue,RuntimeValue> held in a
 * shared_ptr and stored in StructInstance::native_handle.
 */
#include <falcon-typing/FFIHelpers.hpp>
#include <map>
#include <stdexcept>
#include <string>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// ---------------------------------------------------------------------------
// Internal backing type
// ---------------------------------------------------------------------------

// We need RuntimeValue as a map key, which requires strict weak ordering.
// We provide a comparator that uses a canonical string representation.
struct RuntimeValueCmp {
  bool operator()(const RuntimeValue &a, const RuntimeValue &b) const {
    // Compare by (type-index, then value).
    if (a.index() != b.index()) return a.index() < b.index();
    // Same type — compare values.
    if (std::holds_alternative<int64_t>(a))
      return std::get<int64_t>(a) < std::get<int64_t>(b);
    if (std::holds_alternative<double>(a))
      return std::get<double>(a) < std::get<double>(b);
    if (std::holds_alternative<bool>(a))
      return (int)std::get<bool>(a) < (int)std::get<bool>(b);
    if (std::holds_alternative<std::string>(a))
      return std::get<std::string>(a) < std::get<std::string>(b);
    // Non-primitive keys are not supported as map keys.
    throw std::runtime_error("Map: unsupported key type for ordering");
  }
};

using MapStore = std::map<RuntimeValue, RuntimeValue, RuntimeValueCmp>;
using MapStoreSP = std::shared_ptr<MapStore>;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static MapStoreSP get_map(const FalconParamEntry *p, int32_t pc,
                           const char *key = "this") {
  return get_opaque<MapStore>(p, pc, key);
}

static void pack_map(MapStoreSP m, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Map";
  out[0].value.opaque.ptr = new MapStoreSP(std::move(m));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<MapStoreSP *>(p);
  };
  *oc = 1;
}

static void pack_array_rv(std::vector<RuntimeValue> elems,
                           FalconResultSlot *out, int32_t *oc) {
  auto arr = std::make_shared<ArrayValue>();
  arr->elements = std::move(elems);
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr = new std::shared_ptr<ArrayValue>(std::move(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<ArrayValue> *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── New() -> (Map<K,V> m) ─────────────────────────────────────────────────
void STRUCTMapNew(const FalconParamEntry * /*p*/, int32_t /*pc*/,
                  FalconResultSlot *out, int32_t *oc) {
  pack_map(std::make_shared<MapStore>(), out, oc);
}

// ── Size() -> (int size) ──────────────────────────────────────────────────
void STRUCTMapSize(const FalconParamEntry *p, int32_t pc,
                   FalconResultSlot *out, int32_t *oc) {
  auto m = get_map(p, pc);
  pack_results(FunctionResult{static_cast<int64_t>(m->size())}, out, 16, oc);
}

// ── IsEmpty() -> (bool empty) ─────────────────────────────────────────────
void STRUCTMapIsEmpty(const FalconParamEntry *p, int32_t pc,
                      FalconResultSlot *out, int32_t *oc) {
  auto m = get_map(p, pc);
  pack_results(FunctionResult{m->empty()}, out, 16, oc);
}

// ── Get(K key) -> (V value) ───────────────────────────────────────────────
void STRUCTMapGet(const FalconParamEntry *p, int32_t pc,
                  FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto m  = get_map(p, pc);
  auto it = m->find(pm.at("key"));
  if (it == m->end()) {
    throw std::runtime_error("Map.Get: key not found");
  }
  pack_results(FunctionResult{it->second}, out, 16, oc);
}

// ── GetOr(K key, V default_value) -> (V value) ────────────────────────────
void STRUCTMapGetOr(const FalconParamEntry *p, int32_t pc,
                    FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto m  = get_map(p, pc);
  auto it = m->find(pm.at("key"));
  RuntimeValue val = (it != m->end()) ? it->second : pm.at("default_value");
  pack_results(FunctionResult{std::move(val)}, out, 16, oc);
}

// ── Set(K key, V value) -> () ─────────────────────────────────────────────
void STRUCTMapSet(const FalconParamEntry *p, int32_t pc,
                  FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto m  = get_map(p, pc);
  (*m)[pm.at("key")] = pm.at("value");
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Remove(K key) -> (bool removed) ──────────────────────────────────────
void STRUCTMapRemove(const FalconParamEntry *p, int32_t pc,
                     FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto m  = get_map(p, pc);
  bool removed = (m->erase(pm.at("key")) > 0);
  pack_results(FunctionResult{removed}, out, 16, oc);
}

// ── Clear() -> () ─────────────────────────────────────────────────────────
void STRUCTMapClear(const FalconParamEntry *p, int32_t pc,
                    FalconResultSlot *out, int32_t *oc) {
  auto m = get_map(p, pc);
  m->clear();
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Contains(K key) -> (bool found) ──────────────────────────────────────
void STRUCTMapContains(const FalconParamEntry *p, int32_t pc,
                        FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto m  = get_map(p, pc);
  bool found = (m->find(pm.at("key")) != m->end());
  pack_results(FunctionResult{found}, out, 16, oc);
}

// ── Keys() -> (Array<K> keys) ─────────────────────────────────────────────
void STRUCTMapKeys(const FalconParamEntry *p, int32_t pc,
                   FalconResultSlot *out, int32_t *oc) {
  auto m = get_map(p, pc);
  std::vector<RuntimeValue> keys;
  keys.reserve(m->size());
  for (const auto &[k, _] : *m) keys.push_back(k);
  pack_array_rv(std::move(keys), out, oc);
}

// ── Values() -> (Array<V> values) ─────────────────────────────────────────
void STRUCTMapValues(const FalconParamEntry *p, int32_t pc,
                     FalconResultSlot *out, int32_t *oc) {
  auto m = get_map(p, pc);
  std::vector<RuntimeValue> vals;
  vals.reserve(m->size());
  for (const auto &[_, v] : *m) vals.push_back(v);
  pack_array_rv(std::move(vals), out, oc);
}

} // extern "C"
