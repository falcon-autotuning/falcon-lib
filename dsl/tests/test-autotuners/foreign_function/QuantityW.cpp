#include <falcon-typing/FFIHelpers.hpp>
using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

// User-defined struct — NOT a variant member of the engine's RuntimeValue.
// We pass it as FALCON_TYPE_OPAQUE with type_name "Quantity".
struct Quantity {
  int64_t a;
  int64_t b;
  Quantity(int64_t a_, int64_t b_) : a(a_), b(b_) {}
};
using QuantitySP = std::shared_ptr<Quantity>;

// Helper: pack an opaque user struct into a result slot
static void pack_opaque_quantity(QuantitySP q, FalconResultSlot *out,
                                 int32_t *out_count) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name =
      "Quantity"; // user-defined; not a known engine type
  out[0].value.opaque.ptr = new QuantitySP(std::move(q));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<QuantitySP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// New(int a, int b) -> (Quantity q)
void STRUCTQuantityNew(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  int64_t a = std::get<int64_t>(pm.at("a"));
  int64_t b = std::get<int64_t>(pm.at("b"));
  auto q = std::make_shared<Quantity>(a, b);
  pack_opaque_quantity(std::move(q), out, out_count);
}

// NewWithB(int a, int b) -> (Quantity q)  [alias kept for compatibility]
void STRUCTQuantityNewWithB(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *out_count) {
  STRUCTQuantityNew(params, param_count, out, out_count);
}

// Value -> (int value)  [reads field 'a' as the value]
void STRUCTQuantityValue(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *out_count) {
  // 'this' is passed as FALCON_TYPE_OPAQUE with type_name "Quantity"
  QuantitySP q = get_opaque<Quantity>(params, param_count, "this");
  pack_results(FunctionResult{q->a}, out, 16, out_count);
}

} // extern "C"
