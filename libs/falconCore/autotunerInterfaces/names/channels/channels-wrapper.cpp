#include "falcon_core/autotuner_interfaces/names/Channels.hpp"
#include "falcon_core/autotuner_interfaces/names/Channel.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Channels   = falcon_core::autotuner_interfaces::names::Channels;
using ChannelsSP = std::shared_ptr<Channels>;
using Channel    = falcon_core::autotuner_interfaces::names::Channel;
using ChannelSP  = std::shared_ptr<Channel>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_opaque_channels(ChannelsSP chs, FalconResultSlot *out,
                                  int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Channels";
  out[0].value.opaque.ptr       = new ChannelsSP(std::move(chs));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ChannelsSP *>(p);
  };
  *oc = 1;
}

static void pack_opaque_channel(ChannelSP ch, FalconResultSlot *out,
                                 int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Channel";
  out[0].value.opaque.ptr       = new ChannelSP(std::move(ch));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<ChannelSP *>(p);
  };
  *oc = 1;
}

static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) continue;

    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(
          std::string("STRUCTChannelsNew: parameter '") + key +
          "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    }

    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";

    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue") {
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    }

    throw std::runtime_error(
        std::string("STRUCTChannelsNew: parameter '") + key +
        "' has unexpected opaque type_name='" + tn + "'");
  }
  throw std::runtime_error(
      std::string("STRUCTChannelsNew: parameter '") + key + "' not found");
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(array: Array<Channel>) -> (Channels channels)
void STRUCTChannelsNew(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "array");

  std::vector<ChannelSP> vec;
  vec.reserve(arr_val->elements.size());

  for (const auto &elem : arr_val->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem)) {
      throw std::runtime_error(
          "STRUCTChannelsNew: array element is not a StructInstance");
    }
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value()) {
      throw std::runtime_error(
          "STRUCTChannelsNew: Channel StructInstance has no native_handle");
    }
    vec.push_back(std::static_pointer_cast<Channel>(inst->native_handle.value()));
  }

  pack_opaque_channels(std::make_shared<Channels>(vec), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: Channels, other: Channels) -> (bool equal)
void STRUCTChannelsEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Channels>(params, param_count, "this");
  auto other = get_opaque<Channels>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Channels, other: Channels) -> (bool notequal)
void STRUCTChannelsNotEqual(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Channels>(params, param_count, "this");
  auto other = get_opaque<Channels>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Channels) -> (string json)
void STRUCTChannelsToJSON(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Channels channel)
void STRUCTChannelsFromJSON(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto chs  = Channels::from_json_string<Channels>(json);
  pack_opaque_channels(std::make_shared<Channels>(*chs), out, oc);
}

// ── List accessors ────────────────────────────────────────────────────────────

// Size(this: Channels) -> (int size)
void STRUCTChannelsSize(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->size())}, out, 16, oc);
}

// IsEmpty(this: Channels) -> (bool empty)
void STRUCTChannelsIsEmpty(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

// GetIndex(this: Channels, index: int) -> (Channel value)
void STRUCTChannelsGetIndex(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Channels>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto ch = self->at(static_cast<size_t>(idx));
  pack_opaque_channel(ch, out, oc);
}

// SetIndex(this: Channels, index: int, value: Channel) -> ()
void STRUCTChannelsSetIndex(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Channels>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto ch = get_opaque<Channel>(params, param_count, "value");
  self->replace_at(static_cast<size_t>(idx), ch);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// PushBack(this: Channels, value: Channel) -> ()
void STRUCTChannelsPushBack(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  auto ch   = get_opaque<Channel>(params, param_count, "value");
  self->push_back(ch);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// PopBack(this: Channels) -> (Channel value)
void STRUCTChannelsPopBack(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  auto ch   = self->back();
  self->erase_at(self->size() - 1);
  pack_opaque_channel(ch, out, oc);
}

// Insert(this: Channels, index: int, value: Channel) -> ()
void STRUCTChannelsInsert(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Channels>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto ch = get_opaque<Channel>(params, param_count, "value");
  std::vector<ChannelSP> tmp{ch};
  auto it = self->begin();
  std::advance(it, static_cast<size_t>(idx));
  self->insert(it, tmp.begin(), tmp.end());
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Erase(this: Channels, index: int) -> ()
void STRUCTChannelsErase(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto self = get_opaque<Channels>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  self->erase_at(static_cast<size_t>(idx));
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Clear(this: Channels) -> ()
void STRUCTChannelsClear(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  self->clear();
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Contains(this: Channels, value: Channel) -> (bool found)
void STRUCTChannelsContains(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  auto ch   = get_opaque<Channel>(params, param_count, "value");
  pack_results(FunctionResult{self->contains(ch)}, out, 16, oc);
}

// IndexOf(this: Channels, value: Channel) -> (int index)
void STRUCTChannelsIndexOf(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Channels>(params, param_count, "this");
  auto ch   = get_opaque<Channel>(params, param_count, "value");
  pack_results(FunctionResult{static_cast<int64_t>(self->index(ch))}, out, 16, oc);
}

} // extern "C"
