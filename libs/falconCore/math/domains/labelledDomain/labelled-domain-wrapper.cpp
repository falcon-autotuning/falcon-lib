#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/math/domains/Domain.hpp"
#include "falcon_core/math/domains/LabelledDomain.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/physics/units/SymbolUnit.hpp"
#include <falcon-typing/FFIHelpers.hpp>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using LabelledDomain = falcon_core::math::domains::LabelledDomain;
using LabelledDomainSP = std::shared_ptr<LabelledDomain>;
using Domain = falcon_core::math::domains::Domain;
using DomainSP = std::shared_ptr<Domain>;
using InstrumentPort =
    falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP = std::shared_ptr<InstrumentPort>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;
using SymbolUnit = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP = std::shared_ptr<SymbolUnit>;

// ── helpers
// ───────────────────────────────────────────────────────────────────

static void pack_labelled_domain(LabelledDomainSP ld, FalconResultSlot *out,
                                 int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledDomain";
  out[0].value.opaque.ptr = new LabelledDomainSP(std::move(ld));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<LabelledDomainSP *>(p);
  };
  *oc = 1;
}

static void pack_domain(DomainSP d, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Domain";
  out[0].value.opaque.ptr = new DomainSP(std::move(d));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<DomainSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// NewFromKnob(defaultName: string, min: float, max: float,
//             psuedoName: Connection, instrumentType: Instrument,
//             lesserBoundContained: bool, greaterBoundContained: bool,
//             units: SymbolUnit, description: string) -> (LabelledDomain d)
void STRUCTLabelledDomainNewFromKnob(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string default_name = std::get<std::string>(pm.at("defaultName"));
  double min = std::get<double>(pm.at("min"));
  double max = std::get<double>(pm.at("max"));
  auto conn = get_opaque<Connection>(params, param_count, "psuedoName");
  std::string inst_type = std::get<std::string>(pm.at("instrumentType"));
  bool lesser = std::get<bool>(pm.at("lesserBoundContained"));
  bool greater = std::get<bool>(pm.at("greaterBoundContained"));
  auto unit = get_opaque<SymbolUnit>(params, param_count, "units");
  std::string desc = std::get<std::string>(pm.at("description"));
  auto ld = std::make_shared<LabelledDomain>(
      default_name, std::make_pair(min, max), conn, inst_type, lesser, greater,
      unit, desc);
  pack_labelled_domain(std::move(ld), out, oc);
}

// FromPort(max: float, min: float, port: InstrumentPort,
//          lesserboundContainer: bool, greaterBoundContained: bool)
//         -> (LabelledDomain d)
void STRUCTLabelledDomainFromPort(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  double max = std::get<double>(pm.at("max"));
  double min = std::get<double>(pm.at("min"));
  auto port = get_opaque<InstrumentPort>(params, param_count, "port");
  bool lesser = std::get<bool>(pm.at("lesserboundContainer"));
  bool greater = std::get<bool>(pm.at("greaterBoundContained"));
  auto ld = LabelledDomain::from_port(std::make_pair(max, min), port, lesser,
                                      greater);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

// FromPortAndDomain(port: InstrumentPort, domain: Domain) -> (LabelledDomain d)
void STRUCTLabelledDomainFromPortAndDomain(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto port = get_opaque<InstrumentPort>(params, param_count, "port");
  auto domain = get_opaque<Domain>(params, param_count, "domain");
  auto ld = LabelledDomain::from_port_and_domain(port, domain);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

// FromDomain(domain: Domain, defaultName: string, psuedoName: Connection,
//            instrumentType: Instrument, units: SymbolUnit,
//            description: string) -> (LabelledDomain d)
void STRUCTLabelledDomainFromDomain(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto domain = get_opaque<Domain>(params, param_count, "domain");
  std::string default_name = std::get<std::string>(pm.at("defaultName"));
  auto conn = get_opaque<Connection>(params, param_count, "psuedoName");
  std::string inst_type = std::get<std::string>(pm.at("instrumentType"));
  auto unit = get_opaque<SymbolUnit>(params, param_count, "units");
  std::string desc = std::get<std::string>(pm.at("description"));
  auto ld = LabelledDomain::from_domain(domain, default_name, conn, inst_type,
                                        unit, desc);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

// Port(this: LabelledDomain) -> (InstrumentPort port)
void STRUCTLabelledDomainPort(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto port = self->port();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "InstrumentPort";
  out[0].value.opaque.ptr = new InstrumentPortSP(port);
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<InstrumentPortSP *>(p);
  };
  *oc = 1;
}

// Domain(this: LabelledDomain) -> (Domain domain)
void STRUCTLabelledDomainDomain(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_domain(self->domain(), out, oc);
}

// MatchingPort(this: LabelledDomain, port: InstrumentPort) -> (bool matches)
void STRUCTLabelledDomainMatchingPort(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto port = get_opaque<InstrumentPort>(params, param_count, "port");
  pack_results(FunctionResult{self->matching_port(port)}, out, 16, oc);
}

// LesserBound(this: LabelledDomain) -> (float min)
void STRUCTLabelledDomainLesserBound(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->lesser_bound()}, out, 16, oc);
}

// GreaterBound(this: LabelledDomain) -> (float max)
void STRUCTLabelledDomainGreaterBound(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->greater_bound()}, out, 16, oc);
}

// LesserBoundContained(this: LabelledDomain) -> (bool contained)
void STRUCTLabelledDomainLesserBoundContained(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out,
                                              int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->lesser_bound_contained()}, out, 16, oc);
}

// GreaterBoundContained(this: LabelledDomain) -> (bool contained)
void STRUCTLabelledDomainGreaterBoundContained(const FalconParamEntry *params,
                                               int32_t param_count,
                                               FalconResultSlot *out,
                                               int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->greater_bound_contained()}, out, 16, oc);
}

// In(this: LabelledDomain, value: float) -> (bool contains)
void STRUCTLabelledDomainIn(const FalconParamEntry *params, int32_t param_count,
                            FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto pm = unpack_params(params, param_count);
  double value = std::get<double>(pm.at("value"));
  pack_results(FunctionResult{self->in(value)}, out, 16, oc);
}

// Range(this: LabelledDomain) -> (float range)
void STRUCTLabelledDomainRange(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->range()}, out, 16, oc);
}

// GetCenter(this: LabelledDomain) -> (float center)
void STRUCTLabelledDomainGetCenter(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->get_center()}, out, 16, oc);
}

// Intersection(this: LabelledDomain, other: LabelledDomain)
//             -> (LabelledDomain result)
void STRUCTLabelledDomainIntersection(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto other = get_opaque<LabelledDomain>(params, param_count, "other");
  DomainSP result = *self & std::static_pointer_cast<Domain>(other);
  auto ld = LabelledDomain::from_port_and_domain(self->port(), result);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

// Union(this: LabelledDomain, other: LabelledDomain) -> (LabelledDomain result)
void STRUCTLabelledDomainUnion(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto other = get_opaque<LabelledDomain>(params, param_count, "other");
  DomainSP result = *self | std::static_pointer_cast<Domain>(other);
  auto ld = LabelledDomain::from_port_and_domain(self->port(), result);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

// IsEmpty(this: LabelledDomain) -> (bool empty)
void STRUCTLabelledDomainIsEmpty(const FalconParamEntry *params,
                                 int32_t param_count, FalconResultSlot *out,
                                 int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->is_empty()}, out, 16, oc);
}

// ContainsDomain(this: LabelledDomain, other: LabelledDomain) -> (bool
// contains)
void STRUCTLabelledDomainContainsDomain(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto other = get_opaque<LabelledDomain>(params, param_count, "other");
  pack_results(FunctionResult{self->contains_domain(
                   std::static_pointer_cast<Domain>(other))},
               out, 16, oc);
}

// Shift(this: LabelledDomain, offset: float) -> (LabelledDomain shifted)
void STRUCTLabelledDomainShift(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto pm = unpack_params(params, param_count);
  double offset = std::get<double>(pm.at("offset"));
  auto shifted = self->shift(offset);
  auto ld = LabelledDomain::from_port_and_domain(self->port(), shifted);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

// Scaled(this: LabelledDomain, offset: float) -> (LabelledDomain scaled)
void STRUCTLabelledDomainScaled(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto pm = unpack_params(params, param_count);
  double offset = std::get<double>(pm.at("offset"));
  auto scaled = self->scale(offset);
  auto ld = LabelledDomain::from_port_and_domain(self->port(), scaled);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

// Transform(this: LabelledDomain, other: LabelledDomain, value: float)
//          -> (float scale)
void STRUCTLabelledDomainTransform(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto other = get_opaque<LabelledDomain>(params, param_count, "other");
  auto pm = unpack_params(params, param_count);
  double value = std::get<double>(pm.at("value"));
  double result =
      self->transform(std::static_pointer_cast<Domain>(other), value);
  pack_results(FunctionResult{result}, out, 16, oc);
}

// Equal(this: LabelledDomain, other: LabelledDomain) -> (bool equal)
void STRUCTLabelledDomainEqual(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto other = get_opaque<LabelledDomain>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: LabelledDomain, other: LabelledDomain) -> (bool notequal)
void STRUCTLabelledDomainNotEqual(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  auto other = get_opaque<LabelledDomain>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ToJSON(this: LabelledDomain) -> (string json)
void STRUCTLabelledDomainToJSON(const FalconParamEntry *params,
                                int32_t param_count, FalconResultSlot *out,
                                int32_t *oc) {
  auto self = get_opaque<LabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (LabelledDomain dstate)
void STRUCTLabelledDomainFromJSON(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string json = std::get<std::string>(pm.at("json"));
  auto ld = LabelledDomain::from_json_string<LabelledDomain>(json);
  pack_labelled_domain(std::make_shared<LabelledDomain>(*ld), out, oc);
}

} // extern "C"
