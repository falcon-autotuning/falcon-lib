#include "falcon_core/communications/messages/StandardRequest.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using StandardRequest   = falcon_core::communications::messages::StandardRequest;
using StandardRequestSP = std::shared_ptr<StandardRequest>;

static void pack_opaque_standard_request(StandardRequestSP req,
                                         FalconResultSlot  *out,
                                         int32_t           *out_count) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "StandardRequest";
  out[0].value.opaque.ptr       = new StandardRequestSP(std::move(req));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<StandardRequestSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// New(message: string) -> (StandardRequest request)
void STRUCTStandardRequestNew(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *out_count) {
  auto              pm      = unpack_params(params, param_count);
  std::string       message = std::get<std::string>(pm.at("message"));
  StandardRequestSP req     = std::make_shared<StandardRequest>(message);
  pack_opaque_standard_request(std::move(req), out, out_count);
}

// Message(this: StandardRequest) -> (string message)
void STRUCTStandardRequestMessage(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *out_count) {
  StandardRequestSP req = get_opaque<StandardRequest>(params, param_count, "this");
  pack_results(FunctionResult{req->message()}, out, 16, out_count);
}

// Equal(this: StandardRequest, other: StandardRequest) -> (bool equal)
void STRUCTStandardRequestEqual(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *out_count) {
  StandardRequestSP req   = get_opaque<StandardRequest>(params, param_count, "this");
  StandardRequestSP other = get_opaque<StandardRequest>(params, param_count, "other");
  pack_results(FunctionResult{*req == *other}, out, 16, out_count);
}

// NotEqual(this: StandardRequest, other: StandardRequest) -> (bool notequal)
void STRUCTStandardRequestNotEqual(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *out_count) {
  StandardRequestSP req   = get_opaque<StandardRequest>(params, param_count, "this");
  StandardRequestSP other = get_opaque<StandardRequest>(params, param_count, "other");
  pack_results(FunctionResult{*req != *other}, out, 16, out_count);
}

// ToJSON(this: StandardRequest) -> (string json)
void STRUCTStandardRequestToJSON(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *out_count) {
  StandardRequestSP req = get_opaque<StandardRequest>(params, param_count, "this");
  pack_results(FunctionResult{req->to_json_string()}, out, 16, out_count);
}

// FromJSON(json: string) -> (StandardRequest request)
void STRUCTStandardRequestFromJSON(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *out_count) {
  auto              pm   = unpack_params(params, param_count);
  std::string       json = std::get<std::string>(pm.at("json"));
  StandardRequestSP req  = StandardRequest::from_json_string<StandardRequest>(json);
  pack_opaque_standard_request(std::move(req), out, out_count);
}

} // extern "C"
