#include "falcon_core/instrument_interfaces/Waveform.hpp"
#include "falcon_core/instrument_interfaces/port_transforms/PortTransform.hpp"
#include "falcon_core/math/discrete_spaces/DiscreteSpace.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Waveform        = falcon_core::instrument_interfaces::Waveform;
using WaveformSP      = std::shared_ptr<Waveform>;
using PortTransform   = falcon_core::instrument_interfaces::port_transforms::PortTransform;
using PortTransformSP = std::shared_ptr<PortTransform>;
using DiscreteSpace   = falcon_core::math::discrete_spaces::DiscreteSpace;
using DiscreteSpaceSP = std::shared_ptr<DiscreteSpace>;

static void pack_opaque_waveform(WaveformSP w, FalconResultSlot *out,
                                  int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Waveform";
  out[0].value.opaque.ptr       = new WaveformSP(std::move(w));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<WaveformSP *>(p);
  };
  *oc = 1;
}

static void pack_opaque_pt(PortTransformSP pt, FalconResultSlot *out,
                            int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "PortTransform";
  out[0].value.opaque.ptr       = new PortTransformSP(std::move(pt));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PortTransformSP *>(p);
  };
  *oc = 1;
}

static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) continue;
    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE)
      throw std::runtime_error(std::string("get_array_from_params: '") + key + "' not OPAQUE");
    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue")
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    throw std::runtime_error(std::string("get_array_from_params: '") + key + "' unexpected type '" + tn + "'");
  }
  throw std::runtime_error(std::string("get_array_from_params: '") + key + "' not found");
}

extern "C" {

// New(space: DiscreteSpace, transforms: Array<PortTransform>) -> (Waveform waveform)
void STRUCTWaveformNew(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *oc) {
  auto space = get_opaque<DiscreteSpace>(params, param_count, "space");
  auto arr   = get_array_from_params(params, param_count, "transforms");

  auto list = std::make_shared<falcon_core::generic::List<PortTransform>>();
  for (const auto &elem : arr->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem))
      throw std::runtime_error("STRUCTWaveformNew: element is not a StructInstance");
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value())
      throw std::runtime_error("STRUCTWaveformNew: StructInstance has no native_handle");
    auto pt = std::static_pointer_cast<PortTransform>(inst->native_handle.value());
    list->push_back(pt);
  }
  pack_opaque_waveform(std::make_shared<Waveform>(space, list), out, oc);
}

// NewCartesianWaveform1D(division: int, ...) -> stub
void STRUCTWaveformNewCartesianWaveform1D(const FalconParamEntry *, int32_t,
                                          FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTWaveformNewCartesianWaveform1D: not implemented");
}

// NewCartesianIdentityWaveform1D(division: int, ...) -> stub
void STRUCTWaveformNewCartesianIdentityWaveform1D(const FalconParamEntry *, int32_t,
                                                   FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTWaveformNewCartesianIdentityWaveform1D: not implemented");
}

// NewCartesianWaveform(divisions: Axes<int>, ...) -> stub
void STRUCTWaveformNewCartesianWaveform(const FalconParamEntry *, int32_t,
                                        FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTWaveformNewCartesianWaveform: not implemented");
}

// NewCartesianIdentityWaveform(...) -> stub
void STRUCTWaveformNewCartesianIdentityWaveform(const FalconParamEntry *, int32_t,
                                                 FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTWaveformNewCartesianIdentityWaveform: not implemented");
}

// NewCartesianWaveform2D(...) -> stub
void STRUCTWaveformNewCartesianWaveform2D(const FalconParamEntry *, int32_t,
                                          FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTWaveformNewCartesianWaveform2D: not implemented");
}

// NewCartesianIdentityWaveform2D(...) -> stub
void STRUCTWaveformNewCartesianIdentityWaveform2D(const FalconParamEntry *, int32_t,
                                                   FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTWaveformNewCartesianIdentityWaveform2D: not implemented");
}

// Space(this: Waveform) -> (DiscreteSpace space)
void STRUCTWaveformSpace(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto sp   = self->space();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DiscreteSpace";
  out[0].value.opaque.ptr       = new DiscreteSpaceSP(sp);
  out[0].value.opaque.deleter   = [](void *p) { delete static_cast<DiscreteSpaceSP *>(p); };
  *oc = 1;
}

// Size(this: Waveform) -> (int size)
void STRUCTWaveformSize(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->size())}, out, 16, oc);
}

// IsEmpty(this: Waveform) -> (bool empty)
void STRUCTWaveformIsEmpty(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

// GetIndex(this: Waveform, index: int) -> (PortTransform value)
void STRUCTWaveformGetIndex(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto pm   = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  auto list = self->transforms();
  auto it   = list->begin();
  std::advance(it, idx);
  pack_opaque_pt(std::make_shared<PortTransform>(**it), out, oc);
}

// SetIndex(this: Waveform, index: int, value: PortTransform) -> ()
void STRUCTWaveformSetIndex(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto pm   = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  auto list = self->transforms();
  auto it   = list->begin();
  std::advance(it, idx);
  **it = *val;
  *oc = 0;
}

// PushBack(this: Waveform, value: PortTransform) -> ()
void STRUCTWaveformPushBack(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  self->push_back(std::make_shared<PortTransform>(*val));
  *oc = 0;
}

// PopBack(this: Waveform) -> (PortTransform value)
void STRUCTWaveformPopBack(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto list = self->transforms();
  if (list->empty())
    throw std::runtime_error("STRUCTWaveformPopBack: container is empty");
  auto last = list->back();
  self->pop_back();
  pack_opaque_pt(last, out, oc);
}

// Insert(this: Waveform, index: int, value: PortTransform) -> ()
void STRUCTWaveformInsert(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto pm   = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  self->insert(idx, std::make_shared<PortTransform>(*val));
  *oc = 0;
}

// Erase(this: Waveform, index: int) -> ()
void STRUCTWaveformErase(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto pm   = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  self->erase(idx);
  *oc = 0;
}

// Clear(this: Waveform) -> ()
void STRUCTWaveformClear(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  self->clear();
  *oc = 0;
}

// Contains(this: Waveform, value: PortTransform) -> (bool found)
void STRUCTWaveformContains(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  pack_results(FunctionResult{self->contains(*val)}, out, 16, oc);
}

// IndexOf(this: Waveform, value: PortTransform) -> (int index)
void STRUCTWaveformIndexOf(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  pack_results(FunctionResult{static_cast<int64_t>(self->index_of(*val))}, out, 16, oc);
}

// Equal(this: Waveform, other: Waveform) -> (bool equal)
void STRUCTWaveformEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Waveform>(params, param_count, "this");
  auto other = get_opaque<Waveform>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Waveform, other: Waveform) -> (bool notequal)
void STRUCTWaveformNotEqual(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Waveform>(params, param_count, "this");
  auto other = get_opaque<Waveform>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ToJSON(this: Waveform) -> (string json)
void STRUCTWaveformToJSON(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<Waveform>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Waveform waveform)
void STRUCTWaveformFromJSON(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto w    = Waveform::from_json_string<Waveform>(json);
  pack_opaque_waveform(std::make_shared<Waveform>(*w), out, oc);
}

} // extern "C"
