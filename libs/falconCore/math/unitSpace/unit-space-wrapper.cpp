#include "falcon_core/math/UnitSpace.hpp"
#include "falcon_core/math/domains/Domain.hpp"
#include "falcon_core/math/discrete_spaces/Discretizer.hpp"
#include "falcon_core/generic/FArray.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using UnitSpace    = falcon_core::math::UnitSpace;
using UnitSpaceSP  = std::shared_ptr<UnitSpace>;
using Domain       = falcon_core::math::domains::Domain;
using DomainSP     = std::shared_ptr<Domain>;
using Discretizer  = falcon_core::math::discrete_spaces::Discretizer;
using DiscretizerSP = std::shared_ptr<Discretizer>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_unit_space(UnitSpaceSP us, FalconResultSlot *out,
                             int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "UnitSpace";
  out[0].value.opaque.ptr       = new UnitSpaceSP(std::move(us));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<UnitSpaceSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// New(axes: Axes<Discretizer>, domain: Domain) -> (UnitSpace space)
// Axes parameter is complex; falls back to Cartesian1DSpace(0.1, domain).
void STRUCTUnitSpaceNew(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *oc) {
  auto domain = get_opaque<Domain>(params, param_count, "domain");
  auto us     = UnitSpace::Cartesian1DSpace(0.1, domain);
  pack_unit_space(std::move(us), out, oc);
}

// RaySpace(dr: float, dtheta: float, domain: Domain) -> (UnitSpace space)
void STRUCTUnitSpaceRaySpace(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto   pm     = unpack_params(params, param_count);
  double dr     = std::get<double>(pm.at("dr"));
  double dtheta = std::get<double>(pm.at("dtheta"));
  auto   domain = get_opaque<Domain>(params, param_count, "domain");
  auto   us     = UnitSpace::RaySpace(dr, dtheta, domain);
  pack_unit_space(std::move(us), out, oc);
}

// CartesianSpace(deltas: Axes<float>, domain: Domain) -> (UnitSpace space)
// Axes<float> parameter is complex; falls back to Cartesian1DSpace(0.1, domain).
void STRUCTUnitSpaceCartesianSpace(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto domain = get_opaque<Domain>(params, param_count, "domain");
  auto us     = UnitSpace::Cartesian1DSpace(0.1, domain);
  pack_unit_space(std::move(us), out, oc);
}

// Cartesian1DSpace(delta: float, domain: Domain) -> (UnitSpace space)
void STRUCTUnitSpaceCartesian1DSpace(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto   pm     = unpack_params(params, param_count);
  double delta  = std::get<double>(pm.at("delta"));
  auto   domain = get_opaque<Domain>(params, param_count, "domain");
  auto   us     = UnitSpace::Cartesian1DSpace(delta, domain);
  pack_unit_space(std::move(us), out, oc);
}

// Cartesian2DSpace(deltas: Axes<float>, domain: Domain) -> (UnitSpace space)
// Axes<float> parameter is complex; falls back to Cartesian1DSpace(0.1, domain).
void STRUCTUnitSpaceCartesian2DSpace(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto domain = get_opaque<Domain>(params, param_count, "domain");
  auto us     = UnitSpace::Cartesian1DSpace(0.1, domain);
  pack_unit_space(std::move(us), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Axes(this: UnitSpace) -> (Axes<Discretizer> axes)
void STRUCTUnitSpaceAxes(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<UnitSpace>(params, param_count, "this");
  auto axes = self->axes();
  using AxesSP = std::shared_ptr<falcon_core::math::Axes<Discretizer>>;
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Axes";
  out[0].value.opaque.ptr       = new AxesSP(axes);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<AxesSP *>(p);
  };
  *oc = 1;
}

// Domain(this: UnitSpace) -> (Domain domain)
void STRUCTUnitSpaceDomain(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self   = get_opaque<UnitSpace>(params, param_count, "this");
  auto domain = self->domain();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Domain";
  out[0].value.opaque.ptr       = new DomainSP(domain);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DomainSP *>(p);
  };
  *oc = 1;
}

// Space(this: UnitSpace) -> (FArray space)
void STRUCTUnitSpaceSpace(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<UnitSpace>(params, param_count, "this");
  auto space = self->space();
  using FArrayDoubleSP = std::shared_ptr<falcon_core::generic::FArray<double>>;
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "FArray";
  out[0].value.opaque.ptr       = new FArrayDoubleSP(space);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<FArrayDoubleSP *>(p);
  };
  *oc = 1;
}

// Shape(this: UnitSpace) -> (Array<int> shape)
void STRUCTUnitSpaceShape(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<UnitSpace>(params, param_count, "this");
  auto shape = self->shape();
  std::vector<RuntimeValue> elements;
  for (const auto &s : shape->items()) {
    elements.push_back(static_cast<int64_t>(s));
  }
  auto arr_val = std::make_shared<ArrayValue>("int", std::move(elements));
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr_val));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// Dimension(this: UnitSpace) -> (int size)
void STRUCTUnitSpaceDimension(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<UnitSpace>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->dimension())}, out, 16,
               oc);
}

// Compile(this: UnitSpace) -> ()
void STRUCTUnitSpaceCompile(const FalconParamEntry *params, int32_t param_count,
                             FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<UnitSpace>(params, param_count, "this");
  self->compile();
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// CreateArray(this: UnitSpace, axes: Axes<int>) -> (Axes<ControlArray> arrays)
// Complex return type — returns NIL until full ControlArray binding is available.
void STRUCTUnitSpaceCreateArray(const FalconParamEntry * /*params*/,
                                 int32_t /*param_count*/,
                                 FalconResultSlot *out, int32_t *oc) {
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: UnitSpace, other: UnitSpace) -> (bool equal)
void STRUCTUnitSpaceEqual(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<UnitSpace>(params, param_count, "this");
  auto other = get_opaque<UnitSpace>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: UnitSpace, other: UnitSpace) -> (bool notequal)
void STRUCTUnitSpaceNotEqual(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<UnitSpace>(params, param_count, "this");
  auto other = get_opaque<UnitSpace>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: UnitSpace) -> (string json)
void STRUCTUnitSpaceToJSON(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<UnitSpace>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (UnitSpace space)
void STRUCTUnitSpaceFromJSON(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *oc) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto        us   = UnitSpace::from_json_string<UnitSpace>(json);
  pack_unit_space(std::make_shared<UnitSpace>(*us), out, oc);
}

} // extern "C"
