#include "falcon_core/math/AnalyticFunction.hpp"
#include "falcon_core/generic/List.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using AnalyticFunction   = falcon_core::math::AnalyticFunction;
using AnalyticFunctionSP = std::shared_ptr<AnalyticFunction>;
using VariableName       = std::string;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_opaque_af(AnalyticFunctionSP af, FalconResultSlot *out,
                            int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "AnalyticFunction";
  out[0].value.opaque.ptr       = new AnalyticFunctionSP(std::move(af));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<AnalyticFunctionSP *>(p);
  };
  *oc = 1;
}

static void pack_array_result(std::shared_ptr<ArrayValue> arr,
                               FalconResultSlot *out, int32_t *oc) {
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

static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) continue;

    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(
          std::string("STRUCTAnalyticFunctionNew: parameter '") + key +
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
        std::string("STRUCTAnalyticFunctionNew: parameter '") + key +
        "' has unexpected opaque type_name='" + tn + "'");
  }
  throw std::runtime_error(
      std::string("STRUCTAnalyticFunctionNew: parameter '") + key +
      "' not found");
}

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// New(labels: Array<string>, expression: string) -> (AnalyticFunction out)
void STRUCTAnalyticFunctionNew(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  auto arr_val = get_array_from_params(params, param_count, "labels");
  auto expr    = std::get<std::string>(pm.at("expression"));

  auto label_list = std::make_shared<falcon_core::generic::List<VariableName>>();
  for (const auto &elem : arr_val->elements) {
    label_list->push_back(std::get<std::string>(elem));
  }

  auto af = std::make_shared<AnalyticFunction>(label_list, expr);
  pack_opaque_af(std::move(af), out, oc);
}

// Identity() -> (AnalyticFunction out)
void STRUCTAnalyticFunctionIdentity(const FalconParamEntry * /*params*/,
                                     int32_t /*param_count*/,
                                     FalconResultSlot *out, int32_t *oc) {
  auto af = AnalyticFunction::Identity();
  pack_opaque_af(std::move(af), out, oc);
}

// Constant(value: float) -> (AnalyticFunction out)
void STRUCTAnalyticFunctionConstant(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto pm  = unpack_params(params, param_count);
  double val;
  if (std::holds_alternative<double>(pm.at("value"))) {
    val = std::get<double>(pm.at("value"));
  } else {
    val = static_cast<double>(std::get<int64_t>(pm.at("value")));
  }
  auto af = AnalyticFunction::Constant(val);
  pack_opaque_af(std::move(af), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Labels(this: AnalyticFunction) -> (Array<string> variables)
void STRUCTAnalyticFunctionLabels(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self       = get_opaque<AnalyticFunction>(params, param_count, "this");
  auto label_list = self->labels();
  std::vector<RuntimeValue> elements;
  for (const auto &label : *label_list) {
    elements.push_back(label);
  }
  auto arr_val = std::make_shared<ArrayValue>("string", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: AnalyticFunction, other: AnalyticFunction) -> (bool equal)
void STRUCTAnalyticFunctionEqual(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self  = get_opaque<AnalyticFunction>(params, param_count, "this");
  auto other = get_opaque<AnalyticFunction>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: AnalyticFunction, other: AnalyticFunction) -> (bool notequal)
void STRUCTAnalyticFunctionNotEqual(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self  = get_opaque<AnalyticFunction>(params, param_count, "this");
  auto other = get_opaque<AnalyticFunction>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: AnalyticFunction) -> (string json)
void STRUCTAnalyticFunctionToJSON(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self = get_opaque<AnalyticFunction>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (AnalyticFunction dstate)
void STRUCTAnalyticFunctionFromJSON(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto af   = AnalyticFunction::from_json_string<AnalyticFunction>(json);
  pack_opaque_af(std::make_shared<AnalyticFunction>(*af), out, oc);
}

} // extern "C"
