#include "falcon_core/autotuner_interfaces/contexts/MeasurementContext.hpp"
#include "falcon_core/autotuner_interfaces/interpretations/InterpretationContext.hpp"
#include "falcon_core/math/Axes.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using InterpretationContext   = falcon_core::autotuner_interfaces::interpretations::InterpretationContext;
using InterpretationContextSP = std::shared_ptr<InterpretationContext>;
using MeasurementContext      = falcon_core::autotuner_interfaces::contexts::MeasurementContext;
using MeasurementContextSP    = std::shared_ptr<MeasurementContext>;
using SymbolUnit              = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP            = std::shared_ptr<SymbolUnit>;
using MCAxes                  = falcon_core::math::Axes<MeasurementContext>;
using MCAxesSP                = std::shared_ptr<MCAxes>;
using MCList                  = falcon_core::generic::List<MeasurementContext>;
using MCListSP                = std::shared_ptr<MCList>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_ic(InterpretationContextSP ic, FalconResultSlot *out,
                    int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "InterpretationContext";
  out[0].value.opaque.ptr       = new InterpretationContextSP(std::move(ic));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<InterpretationContextSP *>(p);
  };
  *oc = 1;
}

static void pack_mc(MeasurementContextSP mc, FalconResultSlot *out,
                    int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "MeasurementContext";
  out[0].value.opaque.ptr       = new MeasurementContextSP(std::move(mc));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<MeasurementContextSP *>(p);
  };
  *oc = 1;
}

// Extract an ArrayValue from a parameter that is either:
//   - A native Array<T> opaque  (type_name == "Array")
//   - A plain FAL Axes<T> struct (type_name == "StructInstance" with arr_ field)
static std::shared_ptr<ArrayValue>
extract_array_value(const FalconParamEntry *entries, int32_t count,
                    const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0) continue;

    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(std::string("param '") + key +
                               "' is not opaque (tag=" +
                               std::to_string(e.tag) + ")");
    }

    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";

    if (tn == "Array") {
      // Native Array<T>: ptr is shared_ptr<void>* aliasing ArrayValue
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }

    if (tn == "StructInstance") {
      // Pure FAL struct (e.g. Axes<T>) with arr_ field holding native Array
      auto inst =
          *static_cast<std::shared_ptr<StructInstance> *>(e.value.opaque.ptr);
      const auto &arr_field = inst->get_field("arr_");
      auto arr_inst =
          std::get<std::shared_ptr<StructInstance>>(arr_field);
      auto sv = arr_inst->native_handle.value();
      return std::static_pointer_cast<ArrayValue>(sv);
    }

    throw std::runtime_error(std::string("param '") + key +
                             "' has unexpected opaque type: " + tn);
  }
  throw std::runtime_error(std::string("param '") + key + "' not found");
}

// Convert an ArrayValue of MeasurementContext StructInstances to a C++ vector.
static std::vector<MeasurementContextSP>
array_value_to_mc_vector(const std::shared_ptr<ArrayValue> &arr) {
  std::vector<MeasurementContextSP> vec;
  vec.reserve(arr->elements.size());
  for (const auto &elem : arr->elements) {
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    vec.push_back(std::static_pointer_cast<MeasurementContext>(
        inst->native_handle.value()));
  }
  return vec;
}

// Pack a vector of MeasurementContextSP as a native Array opaque.
static void pack_mc_array(const std::vector<MeasurementContextSP> &items,
                          FalconResultSlot *out, int32_t *oc) {
  auto arr = std::make_shared<ArrayValue>("MeasurementContext");
  arr->elements.reserve(items.size());
  for (const auto &mc_sp : items) {
    auto inst          = std::make_shared<StructInstance>("MeasurementContext");
    inst->native_handle = std::static_pointer_cast<void>(mc_sp);
    arr->elements.emplace_back(inst);
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

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(independantVariables: Axes<MeasurementContext>,
//     dependantVariables:   Array<MeasurementContext>,
//     unit:                 SymbolUnit)
//     -> (InterpretationContext context)
void STRUCTInterpretationContextNew(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto indep_arr =
      extract_array_value(params, param_count, "independantVariables");
  auto dep_arr =
      extract_array_value(params, param_count, "dependantVariables");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");

  auto indep_vec = array_value_to_mc_vector(indep_arr);
  auto dep_vec   = array_value_to_mc_vector(dep_arr);

  auto indep_list = std::make_shared<MCList>();
  for (const auto &mc : indep_vec) indep_list->push_back(mc);
  auto axes = std::make_shared<MCAxes>(indep_list);

  auto dep_list = std::make_shared<MCList>();
  for (const auto &mc : dep_vec) dep_list->push_back(mc);

  auto ic = std::make_shared<InterpretationContext>(axes, dep_list, unit);
  pack_ic(std::move(ic), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// IndependentVariables(this: InterpretationContext)
//     -> (Axes<MeasurementContext> contexts)
void STRUCTInterpretationContextIndependentVariables(
    const FalconParamEntry *params, int32_t param_count, FalconResultSlot *out,
    int32_t *oc) {
  auto ic    = get_opaque<InterpretationContext>(params, param_count, "this");
  auto axes  = ic->independent_variables();
  std::vector<MeasurementContextSP> items;
  items.reserve(axes->size());
  for (const auto &mc : *axes) items.push_back(mc);
  pack_mc_array(items, out, oc);
}

// DependentVariables(this: InterpretationContext)
//     -> (Array<MeasurementContext> contexts)
void STRUCTInterpretationContextDependentVariables(
    const FalconParamEntry *params, int32_t param_count, FalconResultSlot *out,
    int32_t *oc) {
  auto ic  = get_opaque<InterpretationContext>(params, param_count, "this");
  auto dep = ic->dependent_variables();
  std::vector<MeasurementContextSP> items;
  items.reserve(dep->size());
  for (const auto &mc : *dep) items.push_back(mc);
  pack_mc_array(items, out, oc);
}

// Unit(this: InterpretationContext) -> (SymbolUnit unit)
void STRUCTInterpretationContextUnit(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto ic   = get_opaque<InterpretationContext>(params, param_count, "this");
  auto unit = ic->unit();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "SymbolUnit";
  out[0].value.opaque.ptr       = new SymbolUnitSP(unit);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<SymbolUnitSP *>(p);
  };
  *oc = 1;
}

// Dimension(this: InterpretationContext) -> (int size)
void STRUCTInterpretationContextDimension(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto ic = get_opaque<InterpretationContext>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(ic->dimension())}, out, 16,
               oc);
}

// ── Mutation ──────────────────────────────────────────────────────────────────

// AddDependentVariable(this: InterpretationContext,
//                      context: MeasurementContext) -> ()
void STRUCTInterpretationContextAddDependentVariable(
    const FalconParamEntry *params, int32_t param_count, FalconResultSlot *out,
    int32_t *oc) {
  auto ic  = get_opaque<InterpretationContext>(params, param_count, "this");
  auto mc  = get_opaque<MeasurementContext>(params, param_count, "context");
  ic->add_dependent_variable(mc);
  out[0]     = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc        = 1;
}

// ReplaceDependantVariable(this: InterpretationContext, index: int,
//                          context: MeasurementContext) -> ()
void STRUCTInterpretationContextReplaceDependantVariable(
    const FalconParamEntry *params, int32_t param_count, FalconResultSlot *out,
    int32_t *oc) {
  auto pm  = unpack_params(params, param_count);
  auto ic  = get_opaque<InterpretationContext>(params, param_count, "this");
  auto mc  = get_opaque<MeasurementContext>(params, param_count, "context");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  ic->replace_dependent_variable(static_cast<int>(idx), mc);
  out[0]     = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc        = 1;
}

// GetIndependentVariable(this: InterpretationContext, index: int)
//     -> (MeasurementContext context)
void STRUCTInterpretationContextGetIndependentVariable(
    const FalconParamEntry *params, int32_t param_count, FalconResultSlot *out,
    int32_t *oc) {
  auto pm  = unpack_params(params, param_count);
  auto ic  = get_opaque<InterpretationContext>(params, param_count, "this");
  int64_t idx = std::get<int64_t>(pm.at("index"));
  auto mc = ic->get_independent_variable(static_cast<int>(idx));
  pack_mc(mc, out, oc);
}

// WithUnit(this: InterpretationContext, unit: SymbolUnit)
//     -> (InterpretationContext context)
void STRUCTInterpretationContextWithUnit(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto ic   = get_opaque<InterpretationContext>(params, param_count, "this");
  auto unit = get_opaque<SymbolUnit>(params, param_count, "unit");
  auto result = ic->with_unit(unit);
  pack_ic(std::const_pointer_cast<InterpretationContext>(result), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: InterpretationContext, other: InterpretationContext) -> (bool equal)
void STRUCTInterpretationContextEqual(const FalconParamEntry *params,
                                       int32_t param_count, FalconResultSlot *out,
                                       int32_t *oc) {
  auto self  = get_opaque<InterpretationContext>(params, param_count, "this");
  auto other = get_opaque<InterpretationContext>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: InterpretationContext, other: InterpretationContext) -> (bool notequal)
void STRUCTInterpretationContextNotEqual(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<InterpretationContext>(params, param_count, "this");
  auto other = get_opaque<InterpretationContext>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: InterpretationContext) -> (string json)
void STRUCTInterpretationContextToJSON(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto ic = get_opaque<InterpretationContext>(params, param_count, "this");
  pack_results(FunctionResult{ic->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (InterpretationContext context)
void STRUCTInterpretationContextFromJSON(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto ic = InterpretationContext::from_json_string<InterpretationContext>(json);
  pack_ic(std::make_shared<InterpretationContext>(*ic), out, oc);
}

} // extern "C"
