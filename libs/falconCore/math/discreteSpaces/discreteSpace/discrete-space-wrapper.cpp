#include "falcon_core/math/discrete_spaces/DiscreteSpace.hpp"
#include "falcon_core/math/domains/CoupledLabelledDomain.hpp"
#include "falcon_core/generic/Map.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using DiscreteSpace   = falcon_core::math::discrete_spaces::DiscreteSpace;
using DiscreteSpaceSP = std::shared_ptr<DiscreteSpace>;
using CoupledLabelledDomain   = falcon_core::math::domains::CoupledLabelledDomain;
using CoupledLabelledDomainSP = std::shared_ptr<CoupledLabelledDomain>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_ds(DiscreteSpaceSP ds, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DiscreteSpace";
  out[0].value.opaque.ptr       = new DiscreteSpaceSP(std::move(ds));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DiscreteSpaceSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: DiscreteSpace, other: DiscreteSpace) -> (bool equal)
void STRUCTDiscreteSpaceEqual(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self  = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto other = get_opaque<DiscreteSpace>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: DiscreteSpace, other: DiscreteSpace) -> (bool notequal)
void STRUCTDiscreteSpaceNotEqual(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self  = get_opaque<DiscreteSpace>(params, param_count, "this");
  auto other = get_opaque<DiscreteSpace>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

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
  auto        pm   = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto        ds   = DiscreteSpace::from_json_string<DiscreteSpace>(json);
  pack_ds(std::make_shared<DiscreteSpace>(*ds), out, oc);
}

// ── Test helpers ──────────────────────────────────────────────────────────────
// NOT for production use. Exposes a minimal 1D cartesian DiscreteSpace so that
// tests can create instances without a full CoupledLabelledDomain constructor
// in the public FAL interface.

// New1D() -> (DiscreteSpace ds)
void STRUCTDiscreteSpaceTestHelpersNew1D(const FalconParamEntry * /*params*/,
                                          int32_t /*param_count*/,
                                          FalconResultSlot *out, int32_t *oc) {
  auto cld = std::make_shared<CoupledLabelledDomain>();
  auto inc = falcon_core::generic::Map<std::string, bool>::create_empty();
  auto ds  = DiscreteSpace::CartesianDiscreteSpace1D(10, cld, inc);
  pack_ds(std::move(ds), out, oc);
}

} // extern "C"
