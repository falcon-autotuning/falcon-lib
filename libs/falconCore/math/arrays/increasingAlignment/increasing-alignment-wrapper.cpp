#include "falcon_core/math/arrays/IncreasingAlignment.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using IncreasingAlignment   = falcon_core::math::arrays::IncreasingAlignment;
using IncreasingAlignmentSP = std::shared_ptr<IncreasingAlignment>;

static void pack_opaque_increasing_alignment(IncreasingAlignmentSP ia,
                                             FalconResultSlot      *out,
                                             int32_t               *out_count) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "IncreasingAlignment";
  out[0].value.opaque.ptr       = new IncreasingAlignmentSP(std::move(ia));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<IncreasingAlignmentSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// New(alignment: bool) -> (IncreasingAlignment inc)
void STRUCTIncreasingAlignmentNew(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *out_count) {
  auto                  pm        = unpack_params(params, param_count);
  bool                  alignment = std::get<bool>(pm.at("alignment"));
  IncreasingAlignmentSP ia        = std::make_shared<IncreasingAlignment>(alignment);
  pack_opaque_increasing_alignment(std::move(ia), out, out_count);
}

// Alignment(this: IncreasingAlignment) -> (bool alignment)
// Returns true when alignment() > 0 (increasing), false otherwise.
void STRUCTIncreasingAlignmentAlignment(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out,
                                        int32_t *out_count) {
  IncreasingAlignmentSP ia = get_opaque<IncreasingAlignment>(params, param_count, "this");
  pack_results(FunctionResult{ia->alignment() > 0}, out, 16, out_count);
}

// Equal(this: IncreasingAlignment, other: IncreasingAlignment) -> (bool equal)
void STRUCTIncreasingAlignmentEqual(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *out_count) {
  IncreasingAlignmentSP ia    = get_opaque<IncreasingAlignment>(params, param_count, "this");
  IncreasingAlignmentSP other = get_opaque<IncreasingAlignment>(params, param_count, "other");
  pack_results(FunctionResult{*ia == *other}, out, 16, out_count);
}

// NotEqual(this: IncreasingAlignment, other: IncreasingAlignment) -> (bool notequal)
void STRUCTIncreasingAlignmentNotEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out,
                                       int32_t *out_count) {
  IncreasingAlignmentSP ia    = get_opaque<IncreasingAlignment>(params, param_count, "this");
  IncreasingAlignmentSP other = get_opaque<IncreasingAlignment>(params, param_count, "other");
  pack_results(FunctionResult{*ia != *other}, out, 16, out_count);
}

// ToJSON(this: IncreasingAlignment) -> (string json)
void STRUCTIncreasingAlignmentToJSON(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *out_count) {
  IncreasingAlignmentSP ia = get_opaque<IncreasingAlignment>(params, param_count, "this");
  pack_results(FunctionResult{ia->to_json_string()}, out, 16, out_count);
}

// FromJSON(json: string) -> (IncreasingAlignment farray)
void STRUCTIncreasingAlignmentFromJSON(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out,
                                       int32_t *out_count) {
  auto                  pm   = unpack_params(params, param_count);
  std::string           json = std::get<std::string>(pm.at("json"));
  IncreasingAlignmentSP ia =
      IncreasingAlignment::from_json_string<IncreasingAlignment>(json);
  pack_opaque_increasing_alignment(std::move(ia), out, out_count);
}

} // extern "C"
