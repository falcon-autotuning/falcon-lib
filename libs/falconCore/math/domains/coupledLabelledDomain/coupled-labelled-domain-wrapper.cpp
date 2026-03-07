#include "falcon_core/math/domains/CoupledLabelledDomain.hpp"
#include "falcon_core/math/domains/LabelledDomain.hpp"
#include "falcon_core/instrument_interfaces/names/Ports.hpp"
#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using CoupledLabelledDomain   = falcon_core::math::domains::CoupledLabelledDomain;
using CoupledLabelledDomainSP = std::shared_ptr<CoupledLabelledDomain>;
using LabelledDomain          = falcon_core::math::domains::LabelledDomain;
using LabelledDomainSP        = std::shared_ptr<LabelledDomain>;
using Ports                   = falcon_core::instrument_interfaces::names::Ports;
using PortsSP                 = std::shared_ptr<Ports>;
using InstrumentPort          = falcon_core::instrument_interfaces::names::InstrumentPort;
using InstrumentPortSP        = std::shared_ptr<InstrumentPort>;

// ── pack helpers ──────────────────────────────────────────────────────────────

static void pack_cld(CoupledLabelledDomainSP cld, FalconResultSlot *out,
                     int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "CoupledLabelledDomain";
  out[0].value.opaque.ptr       = new CoupledLabelledDomainSP(std::move(cld));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<CoupledLabelledDomainSP *>(p);
  };
  *oc = 1;
}

static void pack_labelled_domain(LabelledDomainSP ld, FalconResultSlot *out,
                                  int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledDomain";
  out[0].value.opaque.ptr       = new LabelledDomainSP(std::move(ld));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<LabelledDomainSP *>(p);
  };
  *oc = 1;
}

static RuntimeValue wrap_ld_as_struct(LabelledDomainSP ld) {
  auto inst           = std::make_shared<StructInstance>("LabelledDomain");
  inst->native_handle = std::static_pointer_cast<void>(ld);
  return inst;
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
    if (e.tag != FALCON_TYPE_OPAQUE)
      throw std::runtime_error(std::string("STRUCTCoupledLabelledDomainNew: parameter '") +
                               key + "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue")
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    throw std::runtime_error(std::string("STRUCTCoupledLabelledDomainNew: parameter '") +
                             key + "' has unexpected opaque type_name='" + tn + "'");
  }
  throw std::runtime_error(std::string("STRUCTCoupledLabelledDomainNew: parameter '") +
                           key + "' not found");
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(conns: Array<LabelledDomain>) -> (CoupledLabelledDomain c)
void STRUCTCoupledLabelledDomainNew(const FalconParamEntry *params,
                                     int32_t param_count,
                                     FalconResultSlot *out, int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "conns");
  std::vector<LabelledDomainSP> vec;
  vec.reserve(arr_val->elements.size());
  for (const auto &elem : arr_val->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem))
      throw std::runtime_error(
          "STRUCTCoupledLabelledDomainNew: array element is not a StructInstance");
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value())
      throw std::runtime_error(
          "STRUCTCoupledLabelledDomainNew: LabelledDomain StructInstance has no native_handle");
    vec.push_back(std::static_pointer_cast<LabelledDomain>(inst->native_handle.value()));
  }
  pack_cld(std::make_shared<CoupledLabelledDomain>(vec), out, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this, other) -> (bool equal)
void STRUCTCoupledLabelledDomainEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  auto other = get_opaque<CoupledLabelledDomain>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this, other) -> (bool notequal)
void STRUCTCoupledLabelledDomainNotEqual(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  auto other = get_opaque<CoupledLabelledDomain>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// FromJSON(json: string) -> (CoupledLabelledDomain c)
void STRUCTCoupledLabelledDomainFromJSON(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto cld  = CoupledLabelledDomain::from_json_string<CoupledLabelledDomain>(json);
  pack_cld(std::make_shared<CoupledLabelledDomain>(*cld), out, oc);
}

// ToJSON(this) -> (string json)
void STRUCTCoupledLabelledDomainToJSON(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// ── List accessors ────────────────────────────────────────────────────────────

// Size(this) -> (int size)
void STRUCTCoupledLabelledDomainSize(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->size())}, out, 16, oc);
}

// IsEmpty(this) -> (bool empty)
void STRUCTCoupledLabelledDomainIsEmpty(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

// GetIndex(this, index: int) -> (LabelledDomain value)
void STRUCTCoupledLabelledDomainGetIndex(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto    pm   = unpack_params(params, param_count);
  auto    self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  int64_t idx  = std::get<int64_t>(pm.at("index"));
  auto    ld   = self->at(static_cast<size_t>(idx));
  pack_labelled_domain(ld, out, oc);
}

// PushBack(this, value: LabelledDomain) -> ()
void STRUCTCoupledLabelledDomainPushBack(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  auto ld   = get_opaque<LabelledDomain>(params, param_count, "value");
  self->push_back(ld);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Insert(this, index: int, value: LabelledDomain) -> ()
void STRUCTCoupledLabelledDomainInsert(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto    pm   = unpack_params(params, param_count);
  auto    self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  int64_t idx  = std::get<int64_t>(pm.at("index"));
  auto    ld   = get_opaque<LabelledDomain>(params, param_count, "value");
  std::vector<LabelledDomainSP> tmp{ld};
  auto it = self->begin();
  std::advance(it, static_cast<size_t>(idx));
  self->insert(it, tmp.begin(), tmp.end());
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Contains(this, value: LabelledDomain) -> (bool found)
void STRUCTCoupledLabelledDomainContains(const FalconParamEntry *params,
                                          int32_t param_count,
                                          FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  auto ld   = get_opaque<LabelledDomain>(params, param_count, "value");
  pack_results(FunctionResult{self->contains(ld)}, out, 16, oc);
}

// Index(this, value: LabelledDomain) -> (int index)
void STRUCTCoupledLabelledDomainIndex(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  auto ld   = get_opaque<LabelledDomain>(params, param_count, "value");
  pack_results(FunctionResult{static_cast<int64_t>(self->index(ld))}, out, 16, oc);
}

// Domains(this) -> (Array<LabelledDomain> items)
void STRUCTCoupledLabelledDomainDomains(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  std::vector<RuntimeValue> elements;
  for (const auto &ld : self->domains())
    elements.push_back(wrap_ld_as_struct(ld));
  auto arr_val = std::make_shared<ArrayValue>("LabelledDomain", std::move(elements));
  pack_array_result(arr_val, out, oc);
}

// Labels(this) -> (Ports labels)
void STRUCTCoupledLabelledDomainLabels(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  auto ports = self->labels();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Ports";
  out[0].value.opaque.ptr       = new PortsSP(std::move(ports));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PortsSP *>(p);
  };
  *oc = 1;
}

// GetDomain(this, search: InstrumentPort) -> (LabelledDomain out)
void STRUCTCoupledLabelledDomainGetDomain(const FalconParamEntry *params,
                                           int32_t param_count,
                                           FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  auto port = get_opaque<InstrumentPort>(params, param_count, "search");
  auto ld   = self->get_domain(port);
  pack_labelled_domain(ld, out, oc);
}

// Clear(this) -> ()
void STRUCTCoupledLabelledDomainClear(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  self->clear();
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Erase(this, index: int) -> ()
void STRUCTCoupledLabelledDomainErase(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto    pm   = unpack_params(params, param_count);
  auto    self = get_opaque<CoupledLabelledDomain>(params, param_count, "this");
  int64_t idx  = std::get<int64_t>(pm.at("index"));
  self->erase_at(static_cast<size_t>(idx));
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

} // extern "C"
