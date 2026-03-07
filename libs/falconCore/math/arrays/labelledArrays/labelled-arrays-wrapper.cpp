#include "falcon_core/math/arrays/LabelledArrays.hpp"
#include "falcon_core/math/arrays/LabelledMeasuredArray.hpp"
#include "falcon_core/math/arrays/LabelledControlArray.hpp"
#include "falcon_core/autotuner_interfaces/contexts/AcquisitionContext.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using LMA    = falcon_core::math::arrays::LabelledMeasuredArray;
using LMASP  = std::shared_ptr<LMA>;
using LCA    = falcon_core::math::arrays::LabelledControlArray;
using LCASP  = std::shared_ptr<LCA>;
using LAsM   = falcon_core::math::arrays::LabelledArrays<LMA>;
using LAsMSP = std::shared_ptr<LAsM>;
using LAsC   = falcon_core::math::arrays::LabelledArrays<LCA>;
using LAsCSP = std::shared_ptr<LAsC>;
using AcquisitionContext   = falcon_core::autotuner_interfaces::contexts::AcquisitionContext;
using AcquisitionContextSP = std::shared_ptr<AcquisitionContext>;

// ── Tagged-union wrapper ───────────────────────────────────────────────────────
// LabelledArrays<T> is a template; we monomorphise to Measured and Control
// variants and dispatch at runtime via a tagged union.

struct LabelledArraysWrapper {
  enum class Kind { Measured, Control } kind;
  LAsMSP measured;
  LAsCSP control;

  std::string to_json() const {
    if (kind == Kind::Measured) return measured->to_json_string();
    return control->to_json_string();
  }
  bool eq(const LabelledArraysWrapper& o) const {
    if (kind != o.kind) return false;
    if (kind == Kind::Measured) return *measured == *o.measured;
    return *control == *o.control;
  }
};
using LAW   = LabelledArraysWrapper;
using LAWSP = std::shared_ptr<LAW>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_law(LAWSP law, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledArrays";
  out[0].value.opaque.ptr       = new LAWSP(std::move(law));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<LAWSP *>(p);
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
      throw std::runtime_error(std::string("expected opaque for '") + key + "'");
    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue")
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    throw std::runtime_error(std::string("unexpected opaque type '") + tn + "' for '" + key + "'");
  }
  throw std::runtime_error(std::string("parameter '") + key + "' not found");
}

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// NewLabelledMeasuredArrays(arr: Array<LabelledMeasuredArray>) -> (LabelledArrays arrays)
void STRUCTLabelledArraysNewLabelledMeasuredArrays(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "arr");
  std::vector<LMASP> vec;
  vec.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    auto inst_ptr = std::get_if<std::shared_ptr<StructInstance>>(&elem);
    if (!inst_ptr || !(*inst_ptr) || !(*inst_ptr)->native_handle.has_value())
      throw std::runtime_error(
          "LabelledArraysNewLabelledMeasuredArrays: element is not a valid StructInstance");
    vec.push_back(std::static_pointer_cast<LMA>((*inst_ptr)->native_handle.value()));
  }
  auto law      = std::make_shared<LAW>();
  law->kind     = LAW::Kind::Measured;
  law->measured = LAsM::LabelledMeasuredArrays(vec);
  pack_law(law, out, oc);
}

// NewLabelledControlArrays(arr: Array<LabelledControlArray>) -> (LabelledArrays arrays)
void STRUCTLabelledArraysNewLabelledControlArrays(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "arr");
  std::vector<LCASP> vec;
  vec.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    auto inst_ptr = std::get_if<std::shared_ptr<StructInstance>>(&elem);
    if (!inst_ptr || !(*inst_ptr) || !(*inst_ptr)->native_handle.has_value())
      throw std::runtime_error(
          "LabelledArraysNewLabelledControlArrays: element is not a valid StructInstance");
    vec.push_back(std::static_pointer_cast<LCA>((*inst_ptr)->native_handle.value()));
  }
  auto law     = std::make_shared<LAW>();
  law->kind    = LAW::Kind::Control;
  law->control = LAsC::LabelledControlArrays(vec);
  pack_law(law, out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Arrays(this) -> (Array<T> arrays)
void STRUCTLabelledArraysArrays(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto law = get_opaque<LAW>(params, param_count, "this");
  std::shared_ptr<ArrayValue> arr;
  if (law->kind == LAW::Kind::Measured) {
    arr = std::make_shared<ArrayValue>("LabelledMeasuredArray");
    for (const auto &item : law->measured->items()) {
      auto inst           = std::make_shared<StructInstance>("LabelledMeasuredArray");
      inst->native_handle = std::static_pointer_cast<void>(item);
      arr->elements.push_back(std::move(inst));
    }
  } else {
    arr = std::make_shared<ArrayValue>("LabelledControlArray");
    for (const auto &item : law->control->items()) {
      auto inst           = std::make_shared<StructInstance>("LabelledControlArray");
      inst->native_handle = std::static_pointer_cast<void>(item);
      arr->elements.push_back(std::move(inst));
    }
  }
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// Labels(this) -> (Array<AcquisitionContext> labels)
void STRUCTLabelledArraysLabels(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto law = get_opaque<LAW>(params, param_count, "this");
  auto arr = std::make_shared<ArrayValue>("AcquisitionContext");
  auto pack_labels = [&](const auto &la_sp) {
    for (const auto &ctx : la_sp->labels()->items()) {
      auto inst           = std::make_shared<StructInstance>("AcquisitionContext");
      inst->native_handle = std::static_pointer_cast<void>(ctx);
      arr->elements.push_back(std::move(inst));
    }
  };
  if (law->kind == LAW::Kind::Measured) pack_labels(law->measured);
  else                                  pack_labels(law->control);
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// ── Type predicates ───────────────────────────────────────────────────────────

// IsControlArrays(this) -> (bool IsControl)
void STRUCTLabelledArraysIsControlArrays(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto law    = get_opaque<LAW>(params, param_count, "this");
  bool result = (law->kind == LAW::Kind::Control) && law->control->is_control_arrays();
  pack_results(FunctionResult{result}, out, 16, oc);
}

// IsMeasuredArrays(this) -> (bool IsMeasured)
void STRUCTLabelledArraysIsMeasuredArrays(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto law    = get_opaque<LAW>(params, param_count, "this");
  bool result = (law->kind == LAW::Kind::Measured) && law->measured->is_measured_arrays();
  pack_results(FunctionResult{result}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this, other) -> (bool equal)
void STRUCTLabelledArraysEqual(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LAW>(params, param_count, "this");
  auto other = get_opaque<LAW>(params, param_count, "other");
  pack_results(FunctionResult{self->eq(*other)}, out, 16, oc);
}

// NotEqual(this, other) -> (bool notequal)
void STRUCTLabelledArraysNotEqual(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<LAW>(params, param_count, "this");
  auto other = get_opaque<LAW>(params, param_count, "other");
  pack_results(FunctionResult{!self->eq(*other)}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this) -> (string json)
void STRUCTLabelledArraysToJSON(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto law = get_opaque<LAW>(params, param_count, "this");
  pack_results(FunctionResult{law->to_json()}, out, 16, oc);
}

// FromJSON(json: string) -> (LabelledArrays arrays)
// Attempts to deserialise as LabelledMeasuredArrays first; falls back to
// LabelledControlArrays if the first attempt throws.
void STRUCTLabelledArraysFromJSON(
    const FalconParamEntry *params, int32_t param_count,
    FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto law  = std::make_shared<LAW>();
  try {
    auto m       = LAsM::from_json_string<LAsM>(json);
    law->kind    = LAW::Kind::Measured;
    law->measured = m;
  } catch (...) {
    auto c      = LAsC::from_json_string<LAsC>(json);
    law->kind   = LAW::Kind::Control;
    law->control = c;
  }
  pack_law(law, out, oc);
}

} // extern "C"
