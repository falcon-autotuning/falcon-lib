/**
 * array-wrapper.cpp — FFI implementation for Array<T>.
 *
 * The backing store is falcon::typing::ArrayValue (a std::vector<RuntimeValue>
 * wrapped in a shared_ptr).  The StructInstance for "Array<T>" carries it in
 * native_handle as a shared_ptr<void> aliasing a shared_ptr<ArrayValue>.
 *
 * Pointer convention used throughout this file
 * ────────────────────────────────────────────
 * pack_array emits:
 *   type_name = "Array"
 *   ptr       = new shared_ptr<void>*  (aliasing the ArrayValue)
 *   deleter   = delete (shared_ptr<void>*)p
 *
 * get_array (via get_opaque<ArrayValue>) reads it back as:
 *   auto sv = *static_cast<shared_ptr<void>*>(ptr)
 *   return static_pointer_cast<ArrayValue>(sv)
 *
 * This is the same convention that engine::unpack_results uses for all
 * unknown opaques, and that engine::pack_params uses when forwarding a
 * native StructInstance to a wrapper.  Keeping one consistent convention
 * means the Array can round-trip through the engine without corruption.
 *
 * IMPORTANT: do NOT use  new shared_ptr<ArrayValue>(arr)  here.
 * get_opaque<T> now calls  *static_cast<shared_ptr<void>*>(ptr)  which
 * would reinterpret_cast a shared_ptr<ArrayValue>* as shared_ptr<void>*
 * — those two types have different internal layouts when T != void, giving
 * undefined behaviour.  Always go through shared_ptr<void>.
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
  // get_opaque<ArrayValue> reads ptr as shared_ptr<void>* and
  // static_pointer_cast<ArrayValue> back — see pack_array below.
  return get_opaque<ArrayValue>(p, pc, key);
}

// Pack an ArrayValue result using shared_ptr<void>* so that:
//   (a) engine::unpack_results' else-branch can reinterpret it correctly, and
//   (b) get_opaque<ArrayValue> can static_pointer_cast it back safely.
static void pack_array(std::shared_ptr<ArrayValue> arr, FalconResultSlot *out,
                       int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  // Store as shared_ptr<void>* — the aliasing constructor preserves the
  // ArrayValue* inside the control block so static_pointer_cast<ArrayValue>
  // in get_opaque recovers the correct pointer without UB.
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
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
  pack_results(FunctionResult{static_cast<int64_t>(arr->elements.size())}, out,
               16, oc);
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
  auto pm = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) >= arr->elements.size()) {
    throw std::runtime_error(
        "Array.GetIndex: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  pack_results(FunctionResult{arr->elements[static_cast<size_t>(idx)]}, out, 16,
               oc);
}

// ── SetIndex(int index, T value) -> () ────────────────────────────────────
void STRUCTArraySetIndex(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) >= arr->elements.size()) {
    throw std::runtime_error(
        "Array.SetIndex: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  arr->elements[static_cast<size_t>(idx)] = pm.at("value");
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── PushBack(T value) -> () ───────────────────────────────────────────────
void STRUCTArrayPushBack(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  arr->elements.push_back(pm.at("value"));
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
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
  auto pm = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) > arr->elements.size()) {
    throw std::runtime_error(
        "Array.Insert: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  arr->elements.insert(arr->elements.begin() + idx, pm.at("value"));
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── Erase(int index) -> () ────────────────────────────────────────────────
void STRUCTArrayErase(const FalconParamEntry *p, int32_t pc,
                      FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  int64_t idx = std::get<int64_t>(pm.at("index"));
  if (idx < 0 || static_cast<size_t>(idx) >= arr->elements.size()) {
    throw std::runtime_error(
        "Array.Erase: index out of bounds: " + std::to_string(idx) +
        " (size=" + std::to_string(arr->elements.size()) + ")");
  }
  arr->elements.erase(arr->elements.begin() + idx);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── Clear() -> () ─────────────────────────────────────────────────────────
void STRUCTArrayClear(const FalconParamEntry *p, int32_t pc,
                      FalconResultSlot *out, int32_t *oc) {
  auto arr = get_array(p, pc);
  arr->elements.clear();
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── Contains(T value) -> (bool found) ────────────────────────────────────
void STRUCTArrayContains(const FalconParamEntry *p, int32_t pc,
                         FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  const RuntimeValue &needle = pm.at("value");
  bool found = false;
  for (const auto &elem : arr->elements) {
    if (elem == needle) {
      found = true;
      break;
    }
  }
  pack_results(FunctionResult{found}, out, 16, oc);
}

// ── IndexOf(T value) -> (int index) ──────────────────────────────────────
void STRUCTArrayIndexOf(const FalconParamEntry *p, int32_t pc,
                        FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(p, pc);
  auto arr = get_array(p, pc);
  const RuntimeValue &needle = pm.at("value");
  for (size_t i = 0; i < arr->elements.size(); ++i) {
    // If both are shared_ptr, compare their contents
    if (std::holds_alternative<std::shared_ptr<StructInstance>>(
            arr->elements[i]) &&
        std::holds_alternative<std::shared_ptr<StructInstance>>(needle)) {
      auto lhs = std::get<std::shared_ptr<StructInstance>>(arr->elements[i]);
      auto rhs = std::get<std::shared_ptr<StructInstance>>(needle);
      if (lhs && rhs && *lhs == *rhs) {
        pack_results(FunctionResult{static_cast<int64_t>(i)}, out, 16, oc);
        return;
      }
    } else if (arr->elements[i] == needle) {
      pack_results(FunctionResult{static_cast<int64_t>(i)}, out, 16, oc);
      return;
    }
  }
  pack_results(FunctionResult{static_cast<int64_t>(-1)}, out, 16, oc);
}

} // extern "C"
