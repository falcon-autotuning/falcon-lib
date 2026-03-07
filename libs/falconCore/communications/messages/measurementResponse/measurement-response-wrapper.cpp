#include "falcon_core/communications/messages/MeasurementResponse.hpp"
#include "falcon_core/math/arrays/LabelledArrays.hpp"
#include "falcon_core/math/arrays/LabelledMeasuredArray.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using LabelledMeasuredArray  = falcon_core::math::arrays::LabelledMeasuredArray;
using LabelledMeasuredArrays = falcon_core::math::arrays::LabelledArrays<LabelledMeasuredArray>;
using LabelledMeasuredArraysSP = std::shared_ptr<LabelledMeasuredArrays>;
using MeasurementResponse    = falcon_core::communications::messages::MeasurementResponse;
using MeasurementResponseSP  = std::shared_ptr<MeasurementResponse>;

static void pack_mr(MeasurementResponseSP resp, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "MeasurementResponse";
  out[0].value.opaque.ptr       = new MeasurementResponseSP(std::move(resp));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<MeasurementResponseSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// New(arrays: LabelledArrays<LabelledMeasuredArray>) -> (MeasurementResponse response)
void STRUCTMeasurementResponseNew(const FalconParamEntry *params, int32_t param_count,
                                   FalconResultSlot *out, int32_t *oc) {
  auto arrays = get_opaque<LabelledMeasuredArrays>(params, param_count, "arrays");
  pack_mr(std::make_shared<MeasurementResponse>(arrays), out, oc);
}

// Arrays(this: MeasurementResponse) -> (LabelledArrays<LabelledMeasuredArray> arrays)
void STRUCTMeasurementResponseArrays(const FalconParamEntry *params, int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementResponse>(params, param_count, "this");
  auto arr  = self->arrays();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledMeasuredArrays";
  out[0].value.opaque.ptr       = new LabelledMeasuredArraysSP(arr);
  out[0].value.opaque.deleter   = [](void *p) { delete static_cast<LabelledMeasuredArraysSP *>(p); };
  *oc = 1;
}

// Message(this: MeasurementResponse) -> (string message)
void STRUCTMeasurementResponseMessage(const FalconParamEntry *params, int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementResponse>(params, param_count, "this");
  pack_results(FunctionResult{self->message()}, out, 16, oc);
}

// Equal(this: MeasurementResponse, other: MeasurementResponse) -> (bool equal)
void STRUCTMeasurementResponseEqual(const FalconParamEntry *params, int32_t param_count,
                                     FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasurementResponse>(params, param_count, "this");
  auto other = get_opaque<MeasurementResponse>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: MeasurementResponse, other: MeasurementResponse) -> (bool notequal)
void STRUCTMeasurementResponseNotEqual(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasurementResponse>(params, param_count, "this");
  auto other = get_opaque<MeasurementResponse>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ToJSON(this: MeasurementResponse) -> (string json)
void STRUCTMeasurementResponseToJSON(const FalconParamEntry *params, int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementResponse>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (MeasurementResponse response)
void STRUCTMeasurementResponseFromJSON(const FalconParamEntry *params, int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto resp = MeasurementResponse::from_json_string<MeasurementResponse>(json);
  pack_mr(std::move(resp), out, oc);
}

} // extern "C"
