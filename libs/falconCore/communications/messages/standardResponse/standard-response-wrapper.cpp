#include "falcon_core/communications/messages/StandardResponse.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using StandardResponse   = falcon_core::communications::messages::StandardResponse;
using StandardResponseSP = std::shared_ptr<StandardResponse>;

static void pack_opaque_standard_response(StandardResponseSP resp,
                                          FalconResultSlot   *out,
                                          int32_t            *out_count) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "StandardResponse";
  out[0].value.opaque.ptr       = new StandardResponseSP(std::move(resp));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<StandardResponseSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// New(message: string) -> (StandardResponse response)
void STRUCTStandardResponseNew(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *out_count) {
  auto               pm      = unpack_params(params, param_count);
  std::string        message = std::get<std::string>(pm.at("message"));
  StandardResponseSP resp    = std::make_shared<StandardResponse>(message);
  pack_opaque_standard_response(std::move(resp), out, out_count);
}

// Message(this: StandardResponse) -> (string message)
void STRUCTStandardResponseMessage(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *out_count) {
  StandardResponseSP resp = get_opaque<StandardResponse>(params, param_count, "this");
  pack_results(FunctionResult{resp->message()}, out, 16, out_count);
}

// Equal(this: StandardResponse, other: StandardResponse) -> (bool equal)
void STRUCTStandardResponseEqual(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *out_count) {
  StandardResponseSP resp  = get_opaque<StandardResponse>(params, param_count, "this");
  StandardResponseSP other = get_opaque<StandardResponse>(params, param_count, "other");
  pack_results(FunctionResult{*resp == *other}, out, 16, out_count);
}

// NotEqual(this: StandardResponse, other: StandardResponse) -> (bool notequal)
void STRUCTStandardResponseNotEqual(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *out_count) {
  StandardResponseSP resp  = get_opaque<StandardResponse>(params, param_count, "this");
  StandardResponseSP other = get_opaque<StandardResponse>(params, param_count, "other");
  pack_results(FunctionResult{*resp != *other}, out, 16, out_count);
}

// ToJSON(this: StandardResponse) -> (string json)
void STRUCTStandardResponseToJSON(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *out_count) {
  StandardResponseSP resp = get_opaque<StandardResponse>(params, param_count, "this");
  pack_results(FunctionResult{resp->to_json_string()}, out, 16, out_count);
}

// FromJSON(json: string) -> (StandardResponse response)
void STRUCTStandardResponseFromJSON(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *out_count) {
  auto               pm   = unpack_params(params, param_count);
  std::string        json = std::get<std::string>(pm.at("json"));
  StandardResponseSP resp =
      StandardResponse::from_json_string<StandardResponse>(json);
  pack_opaque_standard_response(std::move(resp), out, out_count);
}

} // extern "C"
