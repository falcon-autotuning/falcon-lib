#include "falcon_core/math/domains/Domain.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;
using Domain   = falcon_core::math::domains::Domain;
using DomainSP = std::shared_ptr<Domain>;

static void pack_opaque_domain(DomainSP d, FalconResultSlot *out,
                               int32_t *out_count) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Domain";
  out[0].value.opaque.ptr       = new DomainSP(std::move(d));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DomainSP *>(p);
  };
  *out_count = 1;
}

extern "C" {

// New(min: float, max: float, lesser_bound_contained: bool,
//     greater_bound_contained: bool) -> (Domain d)
void STRUCTDomainNew(const FalconParamEntry *params, int32_t param_count,
                     FalconResultSlot *out, int32_t *out_count) {
  auto   pm                      = unpack_params(params, param_count);
  double min                     = std::get<double>(pm.at("min"));
  double max                     = std::get<double>(pm.at("max"));
  bool   lesser_bound_contained  = std::get<bool>(pm.at("lesser_bound_contained"));
  bool   greater_bound_contained = std::get<bool>(pm.at("greater_bound_contained"));
  DomainSP d =
      std::make_shared<Domain>(min, max, lesser_bound_contained, greater_bound_contained);
  pack_opaque_domain(std::move(d), out, out_count);
}

// LesserBound(this: Domain) -> (float min)
void STRUCTDomainLesserBound(const FalconParamEntry *params, int32_t param_count,
                              FalconResultSlot *out, int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->lesser_bound()}, out, 16, out_count);
}

// GreaterBound(this: Domain) -> (float max)
void STRUCTDomainGreaterBound(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->greater_bound()}, out, 16, out_count);
}

// LesserBoundContained(this: Domain) -> (bool contained)
void STRUCTDomainLesserBoundContained(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out,
                                      int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->lesser_bound_contained()}, out, 16, out_count);
}

// GreaterBoundContained(this: Domain) -> (bool contained)
void STRUCTDomainGreaterBoundContained(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out,
                                       int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->greater_bound_contained()}, out, 16, out_count);
}

// In(this: Domain, value: float) -> (bool contains)
void STRUCTDomainIn(const FalconParamEntry *params, int32_t param_count,
                    FalconResultSlot *out, int32_t *out_count) {
  DomainSP d     = get_opaque<Domain>(params, param_count, "this");
  auto     pm    = unpack_params(params, param_count);
  double   value = std::get<double>(pm.at("value"));
  pack_results(FunctionResult{d->in(value)}, out, 16, out_count);
}

// Range(this: Domain) -> (float range)
void STRUCTDomainRange(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->range()}, out, 16, out_count);
}

// GetCenter(this: Domain) -> (float center)
void STRUCTDomainGetCenter(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->get_center()}, out, 16, out_count);
}

// Intersection(this: Domain, other: Domain) -> (Domain result)
void STRUCTDomainIntersection(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *out_count) {
  DomainSP d     = get_opaque<Domain>(params, param_count, "this");
  DomainSP other = get_opaque<Domain>(params, param_count, "other");
  DomainSP result = *d & other;
  pack_opaque_domain(std::move(result), out, out_count);
}

// Union(this: Domain, other: Domain) -> (Domain result)
void STRUCTDomainUnion(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *out_count) {
  DomainSP d     = get_opaque<Domain>(params, param_count, "this");
  DomainSP other = get_opaque<Domain>(params, param_count, "other");
  DomainSP result = *d | other;
  pack_opaque_domain(std::move(result), out, out_count);
}

// IsEmpty(this: Domain) -> (bool empty)
void STRUCTDomainIsEmpty(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out, int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->is_empty()}, out, 16, out_count);
}

// ContainsDomain(this: Domain, other: Domain) -> (bool contains)
void STRUCTDomainContainsDomain(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *out_count) {
  DomainSP d     = get_opaque<Domain>(params, param_count, "this");
  DomainSP other = get_opaque<Domain>(params, param_count, "other");
  pack_results(FunctionResult{d->contains_domain(other)}, out, 16, out_count);
}

// Shift(this: Domain, offset: float) -> (Domain shifted)
void STRUCTDomainShift(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *out_count) {
  DomainSP d      = get_opaque<Domain>(params, param_count, "this");
  auto     pm     = unpack_params(params, param_count);
  double   offset = std::get<double>(pm.at("offset"));
  DomainSP result = d->shift(offset);
  pack_opaque_domain(std::move(result), out, out_count);
}

// Scaled(this: Domain, offset: float) -> (Domain scaled)
void STRUCTDomainScaled(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *out_count) {
  DomainSP d      = get_opaque<Domain>(params, param_count, "this");
  auto     pm     = unpack_params(params, param_count);
  double   offset = std::get<double>(pm.at("offset"));
  DomainSP result = d->scale(offset);
  pack_opaque_domain(std::move(result), out, out_count);
}

// Transform(this: Domain, other: Domain, value: float) -> (float scale)
void STRUCTDomainTransform(const FalconParamEntry *params, int32_t param_count,
                           FalconResultSlot *out, int32_t *out_count) {
  DomainSP d     = get_opaque<Domain>(params, param_count, "this");
  DomainSP other = get_opaque<Domain>(params, param_count, "other");
  auto     pm    = unpack_params(params, param_count);
  double   value = std::get<double>(pm.at("value"));
  double   result = d->transform(other, value);
  pack_results(FunctionResult{result}, out, 16, out_count);
}

// Equal(this: Domain, other: Domain) -> (bool equal)
void STRUCTDomainEqual(const FalconParamEntry *params, int32_t param_count,
                       FalconResultSlot *out, int32_t *out_count) {
  DomainSP d     = get_opaque<Domain>(params, param_count, "this");
  DomainSP other = get_opaque<Domain>(params, param_count, "other");
  pack_results(FunctionResult{*d == *other}, out, 16, out_count);
}

// NotEqual(this: Domain, other: Domain) -> (bool notequal)
void STRUCTDomainNotEqual(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *out_count) {
  DomainSP d     = get_opaque<Domain>(params, param_count, "this");
  DomainSP other = get_opaque<Domain>(params, param_count, "other");
  pack_results(FunctionResult{*d != *other}, out, 16, out_count);
}

// ToJSON(this: Domain) -> (string json)
void STRUCTDomainToJSON(const FalconParamEntry *params, int32_t param_count,
                        FalconResultSlot *out, int32_t *out_count) {
  DomainSP d = get_opaque<Domain>(params, param_count, "this");
  pack_results(FunctionResult{d->to_json_string()}, out, 16, out_count);
}

// FromJSON(json: string) -> (Domain dstate)
void STRUCTDomainFromJSON(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *out_count) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  DomainSP    d    = Domain::from_json_string<Domain>(json);
  pack_opaque_domain(std::move(d), out, out_count);
}

} // extern "C"
