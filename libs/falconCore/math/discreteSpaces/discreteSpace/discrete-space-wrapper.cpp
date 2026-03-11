#include "falcon_core/generic/Map.hpp"
#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/instrument_interfaces/names/Ports.hpp"
#include "falcon_core/math/UnitSpace.hpp"
#include "falcon_core/math/arrays/LabelledControlArray.hpp"
#include "falcon_core/math/discrete_spaces/DiscreteSpace.hpp"
#include "falcon_core/math/domains/CoupledLabelledDomain.hpp"
#include "falcon_core/math/domains/Domain.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using DiscreteSpace = falcon_core::math::discrete_spaces::DiscreteSpace;
using DiscreteSpaceSP = std::shared_ptr<DiscreteSpace>;
using CoupledLabelledDomain = falcon_core::math::domains::CoupledLabelledDomain;
using CoupledLabelledDomainSP = std::shared_ptr<CoupledLabelledDomain>;
using UnitSpace = falcon_core::math::UnitSpace;
using UnitSpaceSP = std::shared_ptr<UnitSpace>;
using Domain = falcon_core::math::domains::Domain;
using DomainSP = std::shared_ptr<Domain>;
using InstrumentPort =
    falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP = std::shared_ptr<InstrumentPort>;
using Ports = falcon_core::instrument_interfaces::names::Ports;
using PortsSP = std::shared_ptr<Ports>;
using LabelledControlArray = falcon_core::math::arrays::LabelledControlArray;
using LabelledControlArraySP = std::shared_ptr<LabelledControlArray>;
using BoolMap = falcon_core::generic::Map<std::string, bool>;
using BoolMapSP = std::shared_ptr<BoolMap>;

// ── helpers
// ───────────────────────────────────────────────────────────────────

static void pack_ds(DiscreteSpaceSP ds, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DiscreteSpace";
  out[0].value.opaque.ptr = new DiscreteSpaceSP(std::move(ds));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<DiscreteSpaceSP *>(p);
  };
  *oc = 1;
}

// ---------------------------------------------------------------------------
// get_array_from_params
//   Retrieves a DSL Array<T> / Axes<T> parameter by name.
//   The DSL Array is stored as:
//     FALCON_TYPE_OPAQUE  type_name="Array"
//     ptr = new shared_ptr<void>  (aliasing an ArrayValue)
//   This matches the convention in array-wrapper.cpp / pack_array.
// ---------------------------------------------------------------------------
static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0)
      continue;
    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE)
      throw std::runtime_error(
          std::string("get_array_from_params: parameter '") + key +
          "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
    if (tn == "Array") {
      // Stored as shared_ptr<void>* aliasing an ArrayValue — same as
      // array-wrapper.cpp pack_array / get_opaque<ArrayValue>.
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue")
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    throw std::runtime_error(std::string("get_array_from_params: parameter '") +
                             key + "' has unexpected opaque type_name='" + tn +
                             "'");
  }
  throw std::runtime_error(std::string("get_array_from_params: parameter '") +
                           key + "' not found");
}

// ---------------------------------------------------------------------------
// get_struct_field_array
//   The DSL Map<K,V> is a pure-FAL struct with two named Array fields:
//     keys_   : Array<K>
//     values_ : Array<V>
//   When it arrives as a parameter it is a StructInstance whose named fields
//   are themselves StructInstances wrapping ArrayValues (via the Array
//   shared_ptr<void> convention).  We reach the field arrays by walking the
//   StructInstance's fields map.
//
//   Returns the ArrayValue for the named field.
// ---------------------------------------------------------------------------
static std::shared_ptr<ArrayValue>
get_struct_field_array(const std::shared_ptr<StructInstance> &inst,
                       const char *field_name) {
  if (!inst)
    throw std::runtime_error(
        std::string("get_struct_field_array: null StructInstance for field '") +
        field_name + "'");
  // Fields are stored in inst->fields as RuntimeValue entries keyed by name.
  auto it = inst->fields->find(field_name);
  if (it == inst->fields->end())
    throw std::runtime_error(std::string("get_struct_field_array: field '") +
                             field_name + "' not found in StructInstance");
  const RuntimeValue &fv = it->second;
  // The field itself is an Array, represented as a nested StructInstance
  // carrying native_handle = shared_ptr<void> aliasing an ArrayValue.
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(fv)) {
    auto farr_inst = std::get<std::shared_ptr<StructInstance>>(fv);
    if (farr_inst && farr_inst->native_handle.has_value()) {
      return std::static_pointer_cast<ArrayValue>(
          farr_inst->native_handle.value());
    }
  }
  throw std::runtime_error(std::string("get_struct_field_array: field '") +
                           field_name +
                           "' is not a StructInstance with a native_handle");
}

// ---------------------------------------------------------------------------
// dsl_map_to_bool_map
//   Converts a DSL Map<string, bool> StructInstance into a
//   falcon_core::generic::Map<std::string, bool>.
//
//   The DSL Map<K,V> struct has two Array fields:
//     keys_   : Array<string>  — elements are std::string (RuntimeValue)
//     values_ : Array<bool>    — elements are bool (RuntimeValue)
//   Indices are parallel: keys_[i] corresponds to values_[i].
// ---------------------------------------------------------------------------
static BoolMapSP
dsl_map_to_bool_map(const std::shared_ptr<StructInstance> &inst) {
  auto keys_arr = get_struct_field_array(inst, "keys_");
  auto values_arr = get_struct_field_array(inst, "values_");
  if (keys_arr->elements.size() != values_arr->elements.size()) {
    throw std::runtime_error(
        "dsl_map_to_bool_map: keys_ and values_ have different sizes");
  }
  auto m = std::make_shared<BoolMap>();
  for (size_t i = 0; i < keys_arr->elements.size(); ++i) {
    std::string key = std::get<std::string>(keys_arr->elements[i]);
    bool val = std::get<bool>(values_arr->elements[i]);
    m->insert(key, val);
  }
  return m;
}

// ---------------------------------------------------------------------------
// get_dsl_map_param
//   Retrieves a DSL Map<string,bool> parameter by name from the param list.
//   The DSL Map is a StructInstance (type_name="Map") — no native_handle —
//   whose fields contain the two backing arrays.
// ---------------------------------------------------------------------------
static std::shared_ptr<StructInstance>
get_dsl_struct_param(const FalconParamEntry *entries, int32_t count,
                     const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0)
      continue;
    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE)
      throw std::runtime_error(
          std::string("get_dsl_struct_param: parameter '") + key +
          "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    // Arrived as a StructInstance wrapped in shared_ptr<void>
    auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
    auto inst = std::static_pointer_cast<StructInstance>(sv);
    return inst;
  }
  throw std::runtime_error(std::string("get_dsl_struct_param: parameter '") +
                           key + "' not found");
}

// ---------------------------------------------------------------------------
// build_cld_axes
//   Converts a DSL Axes<CoupledLabelledDomain> (an Array of StructInstances
//   each carrying a native CoupledLabelledDomainSP in native_handle) into
//   a falcon_core::math::AxesSP<CoupledLabelledDomain>.
//   Follows the same StructInstance/native_handle pattern as
//   coupled-labelled-domain-wrapper.cpp STRUCTCoupledLabelledDomainNew.
// ---------------------------------------------------------------------------
static falcon_core::math::AxesSP<CoupledLabelledDomain>
build_cld_axes(const FalconParamEntry *params, int32_t count, const char *key) {
  auto arr = get_array_from_params(params, count, key);
  auto axes =
      std::make_shared<falcon_core::math::Axes<CoupledLabelledDomain>>();
  for (const auto &elem : arr->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem))
      throw std::runtime_error(std::string("build_cld_axes: element in '") +
                               key + "' is not a StructInstance");
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value())
      throw std::runtime_error(std::string("build_cld_axes: element in '") +
                               key + "' StructInstance has no native_handle");
    axes->push_back(std::static_pointer_cast<CoupledLabelledDomain>(
        inst->native_handle.value()));
  }
  return axes;
}

// ---------------------------------------------------------------------------
// build_map_axes
//   Converts a DSL Axes<Map<string,bool>> (an Array of DSL Map StructInstances)
//   into a falcon_core::math::AxesSP<BoolMap>.
//   Each element is a DSL Map StructInstance (pure-FAL, no native_handle),
//   so we call dsl_map_to_bool_map on each one.
// ---------------------------------------------------------------------------
static falcon_core::math::AxesSP<BoolMap>
build_map_axes(const FalconParamEntry *params, int32_t count, const char *key) {
  auto arr = get_array_from_params(params, count, key);
  auto axes = std::make_shared<falcon_core::math::Axes<BoolMap>>();
  for (const auto &elem : arr->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem))
      throw std::runtime_error(std::string("build_map_axes: element in '") +
                               key + "' is not a StructInstance");
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    axes->push_back(dsl_map_to_bool_map(inst));
  }
  return axes;
}

// ---------------------------------------------------------------------------
// build_int_axes
//   Converts a DSL Axes<int> (an Array of int64_t primitives) into a
//   falcon_core::math::AxesSP<int>.
// ---------------------------------------------------------------------------
static falcon_core::math::AxesSP<int>
build_int_axes(const FalconParamEntry *params, int32_t count, const char *key) {
  auto arr = get_array_from_params(params, count, key);
  auto axes = std::make_shared<falcon_core::math::Axes<int>>();
  for (const auto &elem : arr->elements)
    axes->push_back(static_cast<int>(std::get<int64_t>(elem)));
  return axes;
}

// ---------------------------------------------------------------------------
// build_ip_axes
//   Converts a DSL Axes<InstrumentPort> (an Array of StructInstances each
//   carrying a native InstrumentPortSP in native_handle) into a
//   falcon_core::math::AxesSP<InstrumentPort>.
// ---------------------------------------------------------------------------
static falcon_core::math::AxesSP<InstrumentPort>
build_ip_axes(const FalconParamEntry *params, int32_t count, const char *key) {
  auto arr = get_array_from_params(params, count, key);
  auto axes = std::make_shared<falcon_core::math::Axes<InstrumentPort>>();
  for (const auto &elem : arr->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem))
      throw std::runtime_error(std::string("build_ip_axes: element in '") +
                               key + "' is not a StructInstance");
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value())
      throw std::runtime_error(std::string("build_ip_axes: element in '") +
                               key + "' StructInstance has no native_handle");
    axes->push_back(
        std::static_pointer_cast<InstrumentPort>(inst->native_handle.value()));
  }
  return axes;
}

// ---------------------------------------------------------------------------
// pack_lca_axes
//   Packs a falcon_core::math::AxesSP<LabelledControlArray> back out to the
//   DSL as an Array of StructInstances, each carrying a native_handle.
// ---------------------------------------------------------------------------
static void pack_lca_axes(falcon_core::math::AxesSP<LabelledControlArray> axes,
                          FalconResultSlot *out, int32_t *oc) {
  auto arr_val = std::make_shared<ArrayValue>("LabelledControlArray");
  for (const auto &lca : *axes) {
    auto inst = std::make_shared<StructInstance>("LabelledControlArray");
    inst->native_handle = std::static_pointer_cast<void>(
        std::make_shared<LabelledControlArray>(*lca));
    arr_val->elements.push_back(std::move(inst));
  }
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr_val));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

extern "C" {

// ─�� Constructors
// ──────────────────────────────────────────────────────────────

// New(space: UnitSpace,
//     axes: Axes<CoupledLabelledDomain>,
//     increasing: Axes<Map<string,bool>>) -> (DiscreteSpace dspace)
void STRUCTDiscreteSpaceNew(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto space = get_opaque<UnitSpace>(params, param_count, "space");
  auto axes = build_cld_axes(params, param_count, "axes");
  auto increasing = build_map_axes(params, param_count, "increasing");
  pack_ds(std::make_shared<DiscreteSpace>(space, axes, increasing), out, oc);
}

// CartesianDiscreteSpace(divisions: Axes<int>,
//                        axes: Axes<CoupledLabelledDomain>,
//                        increasing: Axes<Map<string,bool>>,
//                        domain: Domain) -> (DiscreteSpace dspace)
void STRUCTDiscreteSpaceCartesianDiscreteSpace(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto divisions = build_int_axes(params, param_count, "divisions");
  auto axes = build_cld_axes(params, param_count, "axes");
  auto increasing = build_map_axes(params, param_count, "increasing");
  auto domain = get_opaque<Domain>(params, param_count, "domain");
  pack_ds(DiscreteSpace::CartesianDiscreteSpace(divisions, axes, increasing,
                                                domain),
          out, oc);
}

// CartesianDiscreteSpace1D(division: int,
//                          sharedDomain: CoupledLabelledDomain,
//                          increasing: Map<string,bool>,
//                          domain: Domain) -> (DiscreteSpace dspace)
//
// NOTE: `increasing` is a single DSL Map<string,bool> (not an Axes),
// so we retrieve it as a StructInstance and convert via dsl_map_to_bool_map.
void STRUCTDiscreteSpaceCartesianDiscreteSpace1D(const FalconParamEntry *params,
                                                 int32_t param_count,
                                                 FalconResultSlot *out,
                                                 int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  int division = static_cast<int>(std::get<int64_t>(pm.at("division")));
  auto sharedDomain =
      get_opaque<CoupledLabelledDomain>(params, param_count, "sharedDomain");
  // `increasing` is a DSL Map<string,bool> StructInstance — read it via its
  // StructInstance fields (keys_ / values_ arrays) and copy into a native Map.
  auto inc_inst = get_dsl_struct_param(params, param_count, "increasing");
  auto increasing = dsl_map_to_bool_map(inc_inst);
  auto domain = get_opaque<Domain>(params, param_count, "domain");
  pack_ds(DiscreteSpace::CartesianDiscreteSpace1D(division, sharedDomain,
                                                  increasing, domain),
          out, oc);
}

// ── Accessors
// ─────────────────────────────────────────────────────────────────

// Space(this: DiscreteSpace) -> (UnitSpace space)
void STRUCTDiscreteSpaceSpace(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto us = self->space();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "UnitSpace";
  out[0].value.opaque.ptr = new UnitSpaceSP(us);
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<UnitSpaceSP *>(p);
  };
  *oc = 1;
}

// Increasing(this: DiscreteSpace) -> (Axes<Map<string,bool>> increasing)
//
// Packs the native BoolMap axes back out as a DSL Array of Map StructInstances.
// Each Map StructInstance is re-built with the two Array fields keys_ / values_
// so the DSL can call .Get(), .Keys() etc. on the returned maps.
void STRUCTDiscreteSpaceIncreasing(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto inc = self->increasing(); // AxesSP<BoolMap>

  auto outer_arr = std::make_shared<ArrayValue>("Map");
  for (const auto &m : *inc) {
    // Build the keys_ ArrayValue
    auto keys_arr = std::make_shared<ArrayValue>("string");
    auto vals_arr = std::make_shared<ArrayValue>("bool");
    for (const auto &kv : *m->items()) {
      keys_arr->elements.push_back(kv->first());
      vals_arr->elements.push_back(kv->second());
    }
    // Wrap each ArrayValue as a StructInstance (matching the Array convention)
    auto keys_inst = std::make_shared<StructInstance>("ArrayValue");
    keys_inst->native_handle = std::static_pointer_cast<void>(keys_arr);
    auto vals_inst = std::make_shared<StructInstance>("ArrayValue");
    vals_inst->native_handle = std::static_pointer_cast<void>(vals_arr);

    // Build the Map StructInstance with keys_ and values_ fields
    auto map_inst = std::make_shared<StructInstance>("Map");
    map_inst->fields->insert(std::make_pair("keys_", keys_inst));
    map_inst->fields->insert(std::make_pair("values_", vals_inst));
    outer_arr->elements.push_back(std::move(map_inst));
  }

  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(outer_arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// Knobs(this: DiscreteSpace) -> (Ports ports)
void STRUCTDiscreteSpaceKnobs(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto ports = self->knobs();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Ports";
  out[0].value.opaque.ptr = new PortsSP(std::move(ports));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<PortsSP *>(p);
  };
  *oc = 1;
}

// GetAxis(this: DiscreteSpace, port: InstrumentPort) -> (int axis)
void STRUCTDiscreteSpaceGetAxis(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto port = get_opaque<InstrumentPort>(params, param_count, "port");
  pack_results(FunctionResult{static_cast<int64_t>(self->get_axis(port))}, out,
               16, oc);
}

// GetDomain(this: DiscreteSpace, port: InstrumentPort) -> (Domain domain)
void STRUCTDiscreteSpaceGetDomain(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto port = get_opaque<InstrumentPort>(params, param_count, "port");
  auto domain = self->get_domain(port);
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Domain";
  out[0].value.opaque.ptr = new DomainSP(std::move(domain));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<DomainSP *>(p);
  };
  *oc = 1;
}

// GetProjection(this: DiscreteSpace,
//               projection: Axes<InstrumentPort>)
//     -> (Axes<LabelledControlArray> out)
void STRUCTDiscreteSpaceGetProjection(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto projection = build_ip_axes(params, param_count, "projection");
  pack_lca_axes(self->get_projection(projection), out, oc);
}

// ── Equality
// ──────────────────────────────────────────────────────────────────

// Equal(this: DiscreteSpace, other: DiscreteSpace) -> (bool equal)
void STRUCTDiscreteSpaceEqual(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto other = get_opaque<DiscreteSpace>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: DiscreteSpace, other: DiscreteSpace) -> (bool notequal)
void STRUCTDiscreteSpaceNotEqual(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto other = get_opaque<DiscreteSpace>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON
// ──────────────────────────────────────────────────────────────────────

// ToJSON(this: DiscreteSpace) -> (string json)
void STRUCTDiscreteSpaceToJSON(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<DiscreteSpace>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (DiscreteSpace dstate)
void STRUCTDiscreteSpaceFromJSON(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto ds = DiscreteSpace::from_json_string<DiscreteSpace>(json);
  pack_ds(std::make_shared<DiscreteSpace>(*ds), out, oc);
}

} // extern "C"
