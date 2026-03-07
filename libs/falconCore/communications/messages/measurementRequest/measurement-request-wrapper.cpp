#include "falcon_core/communications/messages/MeasurementRequest.hpp"
#include "falcon_core/instrument_interfaces/Waveform.hpp"
#include "falcon_core/instrument_interfaces/names/Ports.hpp"
#include "falcon_core/instrument_interfaces/names/InstrumentPort.hpp"
#include "falcon_core/instrument_interfaces/port_transforms/PortTransform.hpp"
#include "falcon_core/math/domains/LabelledDomain.hpp"
#include "falcon_core/generic/Map.hpp"
#include "falcon_core/generic/List.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <stdexcept>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using MeasurementRequest   = falcon_core::communications::messages::MeasurementRequest;
using MeasurementRequestSP = std::shared_ptr<MeasurementRequest>;
using Waveform             = falcon_core::instrument_interfaces::Waveform;
using WaveformSP           = std::shared_ptr<Waveform>;
using Ports                = falcon_core::instrument_interfaces::names::Ports;
using PortsSP              = std::shared_ptr<Ports>;
using InstrumentPort       = falcon_core::instrument_interfaces::names::InstrumentPort;
using PortTransform        = falcon_core::instrument_interfaces::port_transforms::PortTransform;
using LabelledDomain       = falcon_core::math::domains::LabelledDomain;
using LabelledDomainSP     = std::shared_ptr<LabelledDomain>;

static void pack_mr(MeasurementRequestSP req, FalconResultSlot *out, int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "MeasurementRequest";
  out[0].value.opaque.ptr       = new MeasurementRequestSP(std::move(req));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<MeasurementRequestSP *>(p);
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
      throw std::runtime_error(std::string("get_array_from_params: '") + key + "' not OPAQUE");
    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";
    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }
    if (tn == "ArrayValue")
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    throw std::runtime_error(std::string("get_array_from_params: '") + key + "' unexpected type '" + tn + "'");
  }
  throw std::runtime_error(std::string("get_array_from_params: '") + key + "' not found");
}

extern "C" {

// New(message, measurement_name, waveforms, getters, meterTransforms, timeDomain) -> MeasurementRequest
void STRUCTMeasurementRequestNew(const FalconParamEntry *params, int32_t param_count,
                                  FalconResultSlot *out, int32_t *oc) {
  auto pm               = unpack_params(params, param_count);
  auto message          = std::get<std::string>(pm.at("message"));
  auto measurement_name = std::get<std::string>(pm.at("measurement_name"));

  auto arr_val = get_array_from_params(params, param_count, "waveforms");
  auto wlist   = std::make_shared<falcon_core::generic::List<Waveform>>();
  for (const auto &elem : arr_val->elements) {
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem))
      throw std::runtime_error("STRUCTMeasurementRequestNew: waveform element is not StructInstance");
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst || !inst->native_handle.has_value())
      throw std::runtime_error("STRUCTMeasurementRequestNew: StructInstance has no native_handle");
    auto w = std::static_pointer_cast<Waveform>(inst->native_handle.value());
    wlist->push_back(w);
  }

  auto getters     = get_opaque<Ports>(params, param_count, "getters");
  auto time_domain = get_opaque<LabelledDomain>(params, param_count, "timeDomain");

  auto meter_transforms = std::make_shared<
      falcon_core::generic::Map<InstrumentPort, PortTransform>>();

  auto req = std::make_shared<MeasurementRequest>(
      message, measurement_name, wlist, getters, meter_transforms, time_domain);
  pack_mr(std::move(req), out, oc);
}

// MeasurementName(this: MeasurementRequest) -> (string name)
void STRUCTMeasurementRequestMeasurementName(const FalconParamEntry *params,
                                              int32_t param_count,
                                              FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementRequest>(params, param_count, "this");
  pack_results(FunctionResult{self->measurement_name()}, out, 16, oc);
}

// Getters(this: MeasurementRequest) -> (Ports ports)
void STRUCTMeasurementRequestGetters(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementRequest>(params, param_count, "this");
  auto p    = self->getters();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Ports";
  out[0].value.opaque.ptr       = new PortsSP(p);
  out[0].value.opaque.deleter   = [](void *ptr) { delete static_cast<PortsSP *>(ptr); };
  *oc = 1;
}

// Waveforms(this: MeasurementRequest) -> (Array<Waveform> waveforms) - stub
void STRUCTMeasurementRequestWaveforms(const FalconParamEntry *, int32_t,
                                        FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTMeasurementRequestWaveforms: not implemented");
}

// MeterTransforms(this: MeasurementRequest) -> stub
void STRUCTMeasurementRequestMeterTransforms(const FalconParamEntry *, int32_t,
                                              FalconResultSlot *, int32_t *) {
  throw std::runtime_error("STRUCTMeasurementRequestMeterTransforms: not implemented");
}

// TimeDomain(this: MeasurementRequest) -> (LabelledDomain domain)
void STRUCTMeasurementRequestTimeDomain(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementRequest>(params, param_count, "this");
  auto td   = self->time_domain();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "LabelledDomain";
  out[0].value.opaque.ptr       = new LabelledDomainSP(td);
  out[0].value.opaque.deleter   = [](void *p) { delete static_cast<LabelledDomainSP *>(p); };
  *oc = 1;
}

// Message(this: MeasurementRequest) -> (string message)
void STRUCTMeasurementRequestMessage(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementRequest>(params, param_count, "this");
  pack_results(FunctionResult{self->message()}, out, 16, oc);
}

// Equal(this: MeasurementRequest, other: MeasurementRequest) -> (bool equal)
void STRUCTMeasurementRequestEqual(const FalconParamEntry *params,
                                    int32_t param_count,
                                    FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasurementRequest>(params, param_count, "this");
  auto other = get_opaque<MeasurementRequest>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: MeasurementRequest, other: MeasurementRequest) -> (bool notequal)
void STRUCTMeasurementRequestNotEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<MeasurementRequest>(params, param_count, "this");
  auto other = get_opaque<MeasurementRequest>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ToJSON(this: MeasurementRequest) -> (string json)
void STRUCTMeasurementRequestToJSON(const FalconParamEntry *params,
                                     int32_t param_count,
                                     FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<MeasurementRequest>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (MeasurementRequest request)
void STRUCTMeasurementRequestFromJSON(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto req  = MeasurementRequest::from_json_string<MeasurementRequest>(json);
  pack_mr(std::move(req), out, oc);
}

} // extern "C"
