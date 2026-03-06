/**
 * array-wrapper.cpp — FFI implementation for Array<T>.
 *
 * The backing store is falcon::typing::ArrayValue (a std::vector<RuntimeValue>
 * wrapped in a shared_ptr).  The StructInstance for "Array<T>" carries it in
 * native_handle as a shared_ptr<void> aliasing a shared_ptr<ArrayValue>.
 */
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>
#include <string>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::shared_ptr<ArrayValue>
get_array(const FalconParamEntry *p, int32_t pc, const char *key = "this") {
  // The engine packed the StructInstance's native_handle as a shared_ptr<void>*
  // pointing to the underlying ArrayValue shared control block.
  // get_opaque<ArrayValue> casts it back.
  return get_opaque<ArrayValue>(p, pc, key);
}

static void pack_array(std::shared_ptr<ArrayValue> arr,
                       FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<ArrayValue>(std::move(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<ArrayValue> *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── New() -> (Array<T> arr) ────────────────────────────────────────────────
void STRUCTArrayNew(const FalconParamEntry * /*p*/, int32_t /*pc*/,
                    FalconResultSlot *out, int32_t *oc) {
  pack_array(std::make_shared<ArrayValue>(), out, oc);
}

// ── Size() -> (int size) ───────────────────────────────────────────────────
void STRUCTArraySize(const FalconParamEntry *p, int32_t pc,
                     FalconResultSlot *out, int32_t *oc) {
  auto arr = get_array(p, pc);
  pack_results(FunctionResult{static_cast<int64_t>(arr->elements.size())},
               out, 16, oc);
}

// ── IsEmpty() -> (bool empty) ─────────────────────────────────────────────
void STRUCTArrayIsEmpty(const FalconParamEntry *p, int32_t pc,
                        FalconResultSlot *out, int32_t *oc) {
  auto arr = get_array(p, pc);
  pack_results(FunctionResult{arr->elements.empty()}, out, 16, oc);
}

// ── GetIndex(int index) -> (T value) ─────────────────────────────────────
void STRUCTArrayGetIndex(const FalconParamEntry *p, int32_t pc,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm  = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) >= arr->elements.size()) {
    throw std::runtime_error(
        "Array.GetIndex: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  pack_results(FunctionResult{arr->elements[static_cast<size_t>(idx)]},
               out, 16, oc);
}

// ── SetIndex(int index, T value) -> () ────────────────────────────────────
void STRUCTArraySetIndex(const FalconParamEntry *p, int32_t pc,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm  = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) >= arr->elements.size()) {
    throw std::runtime_error(
        "Array.SetIndex: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  arr->elements[static_cast<size_t>(idx)] = pm.at("value");
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── PushBack(T value) -> () ───────────────────────────────────────────────
void STRUCTArrayPushBack(const FalconParamEntry *p, int32_t pc,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm  = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  arr->elements.push_back(pm.at("value"));
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── PopBack() -> (T value) ────────────────────────────────────────────────
void STRUCTArrayPopBack(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
  auto arr = get_array(p, pc);
  if (arr->elements.empty()) {
    throw std::runtime_error("Array.PopBack: array is empty");
  }
  RuntimeValue val = arr->elements.back();
  arr->elements.pop_back();
  pack_results(FunctionResult{std::move(val)}, out, 16, oc);
}

// ── Insert(int index, T value) -> () ─────────────────────────────────────
void STRUCTArrayInsert(const FalconParamEntry *p, int32_t pc,
                        FalconResultSlot *out, int32_t *oc) {
  auto pm  = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) > arr->elements.size()) {
    throw std::runtime_error(
        "Array.Insert: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  arr->elements.insert(arr->elements.begin() + idx, pm.at("value"));
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Erase(int index) -> () ────────────────────────────────────────────────
void STRUCTArrayErase(const FalconParamEntry *p, int32_t pc,
                       FalconResultSlot *out, int32_t *oc) {
  auto pm  = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) >= arr->elements.size()) {
    throw std::runtime_error(
        "Array.Erase: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  arr->elements.erase(arr->elements.begin() + idx);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Clear() -> () ─────────────────────────────────────────────────────────
void STRUCTArrayClear(const FalconParamEntry *p, int32_t pc,
                       FalconResultSlot *out, int32_t *oc) {
  auto arr = get_array(p, pc);
  arr->elements.clear();
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Contains(T value) -> (bool found) ────────────────────────────────────
void STRUCTArrayContains(const FalconParamEntry *p, int32_t pc,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm  = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  const RuntimeValue &needle = pm.at("value");
  bool found = false;
  for (const auto &elem : arr->elements) {
    if (elem == needle) { found = true; break; }
  }
  pack_results(FunctionResult{found}, out, 16, oc);
}

// ── IndexOf(T value) -> (int index) ──────────────────────────────────────
void STRUCTArrayIndexOf(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm  = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  const RuntimeValue &needle = pm.at("value");
  for (size_t i = 0; i < arr->elements.size(); ++i) {
    if (arr->elements[i] == needle) {
      pack_results(FunctionResult{static_cast<int64_t>(i)}, out, 16, oc);
      return;
    }
  }
  pack_results(FunctionResult{static_cast<int64_t>(-1)}, out, 16, oc);
}

} // extern "C"
