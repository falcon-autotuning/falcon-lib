#include "falcon_core/autotuner_interfaces/names/Channel.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using Channel   = falcon_core::autotuner_interfaces::names::Channel;
using ChannelSP = std::shared_ptr<Channel>;

static void pack_opaque_channel(ChannelSP ch, FalconResultSlot *out,
                                int32_t *out_count) {
  out[0]                         = {};
  out[0].tag                     = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name  = "Channel";
  out[0].value.opaque.ptr        = new ChannelSP(std::move(ch));
  out[0].value.opaque.deleter    = [](void *p) {
    delete static_cast<ChannelSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// New(name: string) -> (Channel channel)
void STRUCTChannelNew(const FalconParamEntry *params, int32_t param_count,
                      FalconResultSlot *out, int32_t *out_count) {
  auto        pm   = unpack_params(params, param_count);
  std::string name = std::get<std::string>(pm.at("name"));
  ChannelSP   ch   = std::make_shared<Channel>(name);
  pack_opaque_channel(std::move(ch), out, out_count);
}

// Name(this: Channel) -> (string name)
void STRUCTChannelName(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *out_count) {
  ChannelSP ch = get_opaque<Channel>(params, param_count, "this");
  pack_results(FunctionResult{ch->name()}, out, 16, out_count);
}

// Equal(this: Channel, other: Channel) -> (bool equal)
void STRUCTChannelEqual(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *out_count) {
  ChannelSP ch    = get_opaque<Channel>(params, param_count, "this");
  ChannelSP other = get_opaque<Channel>(params, param_count, "other");
  pack_results(FunctionResult{*ch == *other}, out, 16, out_count);
}

// NotEqual(this: Channel, other: Channel) -> (bool notequal)
void STRUCTChannelNotEqual(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *out_count) {
  ChannelSP ch    = get_opaque<Channel>(params, param_count, "this");
  ChannelSP other = get_opaque<Channel>(params, param_count, "other");
  pack_results(FunctionResult{*ch != *other}, out, 16, out_count);
}

// ToJSON(this: Channel) -> (string json)
void STRUCTChannelToJSON(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *out_count) {
  ChannelSP ch = get_opaque<Channel>(params, param_count, "this");
  pack_results(FunctionResult{ch->to_json_string()}, out, 16, out_count);
}

// FromJSON(json: string) -> (Channel channel)
void STRUCTChannelFromJSON(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *out_count) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  ChannelSP   ch   = Channel::from_json_string<Channel>(json);
  pack_opaque_channel(std::move(ch), out, out_count);
}

} // extern "C"
