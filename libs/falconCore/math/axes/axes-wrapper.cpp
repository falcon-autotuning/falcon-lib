#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// Axes<T> is a FAL struct whose methods are implemented inline in axes.fal.
// Only Equal, NotEqual, FromJSON, and ToJSON are dispatched through FFI.
// Since the FAL runtime passes a struct instance (not a C++ opaque) for `this`,
// these functions provide safe stub implementations.

extern "C" {

// Equal(this: Axes<T>, other: Axes<T>) -> (bool equal)
void STRUCTAxesEqual(const FalconParamEntry *, int32_t,
                     FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{false}, out, 16, oc);
}

// NotEqual(this: Axes<T>, other: Axes<T>) -> (bool notequal)
void STRUCTAxesNotEqual(const FalconParamEntry *, int32_t,
                        FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{true}, out, 16, oc);
}

// ToJSON(this: Axes<T>) -> (string json)
void STRUCTAxesToJSON(const FalconParamEntry *, int32_t,
                      FalconResultSlot *out, int32_t *oc) {
  pack_results(FunctionResult{std::string("{}")}, out, 16, oc);
}

// FromJSON(json: string) -> (Axes<T> axes)
void STRUCTAxesFromJSON(const FalconParamEntry *, int32_t,
                        FalconResultSlot *out, int32_t *oc) {
  out[0]     = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc        = 1;
}

} // extern "C"
