#include "falcon_core/instrument_interfaces/port_transforms/PortTransforms.hpp"
#include "falcon_core/instrument_interfaces/port_transforms/PortTransform.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using PortTransforms   = falcon_core::instrument_interfaces::port_transforms::PortTransforms;
using PortTransformsSP = std::shared_ptr<PortTransforms>;
using PortTransform    = falcon_core::instrument_interfaces::port_transforms::PortTransform;
using PortTransformSP  = std::shared_ptr<PortTransform>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_opaque_pts(PortTransformsSP pts, FalconResultSlot *out,
                             int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "PortTransforms";
  out[0].value.opaque.ptr       = new PortTransformsSP(std::move(pts));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PortTransformsSP *>(p);
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

// Recover shared_ptr<ArrayValue> from an opaque Array parameter.
static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) continue;
    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(
          std::string("get_array_from_params: parameter '") + key +
          "' is not OPAQUE");
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
        std::string("get_array_from_params: parameter '") + key +
        "' has unexpected opaque type_name='" + tn + "'");
  }
  throw std::runtime_error(
      std::string("get_array_from_params: parameter '") + key + "' not found");
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(transforms: Array<PortTransform>) -> (PortTransforms transforms)
void STRUCTPortTransformsNew(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "transforms");

  std::vector<PortTransformSP> vec;
  vec.reserve(arr_val->elements.size());

  for (const auto &elem : arr_val->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem)) {
      throw std::runtime_error(
          "STRUCTPortTransformsNew: array element is not a StructInstance");
    }
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value()) {
      throw std::runtime_error(
          "STRUCTPortTransformsNew: StructInstance has no native_handle");
    }
    auto pt = std::static_pointer_cast<PortTransform>(inst->native_handle.value());
    if (!pt) {
      throw std::runtime_error(
          "STRUCTPortTransformsNew: static_pointer_cast<PortTransform> returned null");
    }
    vec.push_back(std::move(pt));
  }

  pack_opaque_pts(std::make_shared<PortTransforms>(vec), out, oc);
}

// ── List operations ───────────────────────────────────────────────────────────

// GetIndex(this: PortTransforms, index: int) -> (PortTransform value)
void STRUCTPortTransformsGetIndex(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self  = get_opaque<PortTransforms>(params, param_count, "this");
  auto pm    = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  auto list  = self->transforms();
  auto it    = list->begin();
  std::advance(it, idx);
  pack_opaque_pt(std::make_shared<PortTransform>(**it), out, oc);
}

// SetIndex(this: PortTransforms, index: int, value: PortTransform) -> ()
void STRUCTPortTransformsSetIndex(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self  = get_opaque<PortTransforms>(params, param_count, "this");
  auto pm    = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  auto val   = get_opaque<PortTransform>(params, param_count, "value");
  auto list  = self->transforms();
  auto it    = list->begin();
  std::advance(it, idx);
  **it = *val;
  *oc = 0;
}

// PushBack(this: PortTransforms, value: PortTransform) -> ()
void STRUCTPortTransformsPushBack(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  self->push_back(std::make_shared<PortTransform>(*val));
  *oc = 0;
}

// PopBack(this: PortTransforms) -> (PortTransform value)
void STRUCTPortTransformsPopBack(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  auto list = self->transforms();
  if (list->empty()) {
    throw std::runtime_error("STRUCTPortTransformsPopBack: container is empty");
  }
  auto last = list->back();
  self->pop_back();
  pack_opaque_pt(last, out, oc);
}

// Insert(this: PortTransforms, index: int, value: PortTransform) -> ()
void STRUCTPortTransformsInsert(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto self  = get_opaque<PortTransforms>(params, param_count, "this");
  auto pm    = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  auto val   = get_opaque<PortTransform>(params, param_count, "value");
  self->insert(idx, std::make_shared<PortTransform>(*val));
  *oc = 0;
}

// Erase(this: PortTransforms, index: int) -> ()
void STRUCTPortTransformsErase(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self  = get_opaque<PortTransforms>(params, param_count, "this");
  auto pm    = unpack_params(params, param_count);
  int32_t idx = static_cast<int32_t>(std::get<int64_t>(pm.at("index")));
  self->erase(idx);
  *oc = 0;
}

// Clear(this: PortTransforms) -> ()
void STRUCTPortTransformsClear(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  self->clear();
  *oc = 0;
}

// Contains(this: PortTransforms, value: PortTransform) -> (bool found)
void STRUCTPortTransformsContains(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  bool found = self->contains(*val);
  pack_results(FunctionResult{found}, out, 16, oc);
}

// IndexOf(this: PortTransforms, value: PortTransform) -> (int index)
void STRUCTPortTransformsIndexOf(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  auto val  = get_opaque<PortTransform>(params, param_count, "value");
  int64_t idx = static_cast<int64_t>(self->index_of(*val));
  pack_results(FunctionResult{idx}, out, 16, oc);
}

// Size(this: PortTransforms) -> (int size)
void STRUCTPortTransformsSize(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  int64_t sz = static_cast<int64_t>(self->size());
  pack_results(FunctionResult{sz}, out, 16, oc);
}

// IsEmpty(this: PortTransforms) -> (bool empty)
void STRUCTPortTransformsIsEmpty(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: PortTransforms, other: PortTransforms) -> (bool equal)
void STRUCTPortTransformsEqual(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self  = get_opaque<PortTransforms>(params, param_count, "this");
  auto other = get_opaque<PortTransforms>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: PortTransforms, other: PortTransforms) -> (bool notequal)
void STRUCTPortTransformsNotEqual(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self  = get_opaque<PortTransforms>(params, param_count, "this");
  auto other = get_opaque<PortTransforms>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: PortTransforms) -> (string json)
void STRUCTPortTransformsToJSON(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto self = get_opaque<PortTransforms>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (PortTransforms transforms)
void STRUCTPortTransformsFromJSON(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto p    = PortTransforms::from_json_string<PortTransforms>(json);
  pack_opaque_pts(std::make_shared<PortTransforms>(*p), out, oc);
}

} // extern "C"
