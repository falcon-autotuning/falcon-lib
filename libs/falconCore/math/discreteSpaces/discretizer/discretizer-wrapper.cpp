#include "falcon_core/math/discrete_spaces/Discretizer.hpp"
#include "falcon_core/math/domains/Domain.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using Discretizer   = falcon_core::math::discrete_spaces::Discretizer;
using DiscretizerSP = std::shared_ptr<Discretizer>;
using Domain        = falcon_core::math::domains::Domain;
using DomainSP      = std::shared_ptr<Domain>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_discretizer(DiscretizerSP d, FalconResultSlot *out,
                             int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Discretizer";
  out[0].value.opaque.ptr       = new DiscretizerSP(std::move(d));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DiscretizerSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructors ──────────────────────────────────────────────────────────────

// NewCartesianDiscretizer(delta: float) -> (Discretizer disc)
void STRUCTDiscretizerNewCartesianDiscretizer(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto   pm    = unpack_params(params, param_count);
  double delta = std::get<double>(pm.at("delta"));
  auto   d     = Discretizer::CartesianDiscretizer(delta);
  pack_discretizer(std::move(d), out, oc);
}

// NewPolarDiscretizer(delta: float) -> (Discretizer disc)
void STRUCTDiscretizerNewPolarDiscretizer(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto   pm    = unpack_params(params, param_count);
  double delta = std::get<double>(pm.at("delta"));
  auto   d     = Discretizer::PolarDiscretizer(delta);
  pack_discretizer(std::move(d), out, oc);
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// Delta(this: Discretizer) -> (float d)
void STRUCTDiscretizerDelta(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto d = get_opaque<Discretizer>(params, param_count, "this");
  pack_results(FunctionResult{d->delta()}, out, 16, oc);
}

// SetDelta(this: Discretizer, delta: float) -> ()
void STRUCTDiscretizerSetDelta(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto   pm    = unpack_params(params, param_count);
  auto   d     = get_opaque<Discretizer>(params, param_count, "this");
  double delta = std::get<double>(pm.at("delta"));
  d->set_delta(delta);
  out[0]     = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc        = 1;
}

// Domain(this: Discretizer) -> (Domain d)
void STRUCTDiscretizerDomain(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto      d      = get_opaque<Discretizer>(params, param_count, "this");
  DomainSP  domain = d->domain();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Domain";
  out[0].value.opaque.ptr       = new DomainSP(domain);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DomainSP *>(p);
  };
  *oc = 1;
}

// ── Predicates ────────────────────────────────────────────────────────────────

// IsCartesian(this: Discretizer) -> (bool IsCartesian)
void STRUCTDiscretizerIsCartesian(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto d = get_opaque<Discretizer>(params, param_count, "this");
  pack_results(FunctionResult{d->is_cartesian()}, out, 16, oc);
}

// IsPolar(this: Discretizer) -> (bool IsPolar)
void STRUCTDiscretizerIsPolar(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto d = get_opaque<Discretizer>(params, param_count, "this");
  pack_results(FunctionResult{d->is_polar()}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: Discretizer, other: Discretizer) -> (bool equal)
void STRUCTDiscretizerEqual(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<Discretizer>(params, param_count, "this");
  auto other = get_opaque<Discretizer>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: Discretizer, other: Discretizer) -> (bool notequal)
void STRUCTDiscretizerNotEqual(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self  = get_opaque<Discretizer>(params, param_count, "this");
  auto other = get_opaque<Discretizer>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: Discretizer) -> (string json)
void STRUCTDiscretizerToJSON(const FalconParamEntry *params,
                             int32_t param_count, FalconResultSlot *out,
                             int32_t *oc) {
  auto d = get_opaque<Discretizer>(params, param_count, "this");
  pack_results(FunctionResult{d->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (Discretizer dstate)
void STRUCTDiscretizerFromJSON(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto        d    = Discretizer::from_json_string<Discretizer>(json);
  pack_discretizer(std::make_shared<Discretizer>(*d), out, oc);
}

} // extern "C"
