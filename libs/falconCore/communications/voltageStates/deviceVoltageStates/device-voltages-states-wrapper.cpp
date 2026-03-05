#include "falcon_core/communications/voltage_states/DeviceVoltageStates.hpp"
#include "falcon_core/communications/voltage_states/DeviceVoltageState.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include "falcon_core/math/Point.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using DeviceVoltageStates   = falcon_core::communications::voltage_states::DeviceVoltageStates;
using DeviceVoltageStatesSP = std::shared_ptr<DeviceVoltageStates>;
using DeviceVoltageState    = falcon_core::communications::voltage_states::DeviceVoltageState;
using DeviceVoltageStateSP  = std::shared_ptr<DeviceVoltageState>;
using Connection            = falcon_core::physics::device_structures::Connection;
using ConnectionSP          = std::shared_ptr<Connection>;
using Point                 = falcon_core::math::Point;
using PointSP               = std::shared_ptr<Point>;

// ── helpers ───────────────────────────────────────────────────────────────────

static void pack_dvss(DeviceVoltageStatesSP dvss, FalconResultSlot *out,
                      int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceVoltageStates";
  out[0].value.opaque.ptr       = new DeviceVoltageStatesSP(std::move(dvss));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DeviceVoltageStatesSP *>(p);
  };
  *oc = 1;
}

static void pack_dvs(DeviceVoltageStateSP dvs, FalconResultSlot *out,
                     int32_t *oc) {
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceVoltageState";
  out[0].value.opaque.ptr       = new DeviceVoltageStateSP(std::move(dvs));
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<DeviceVoltageStateSP *>(p);
  };
  *oc = 1;
}

extern "C" {

// ── Constructor ───────────────────────────────────────────────────────────────

// New(states: Array<DeviceVoltageState>) -> (DeviceVoltageStates states)
void STRUCTDeviceVoltageStatesNew(const FalconParamEntry *params,
                                   int32_t param_count, FalconResultSlot *out,
                                   int32_t *oc) {
  auto pm             = unpack_params(params, param_count);
  const auto &arr_val = std::get<ArrayValue>(pm.at("states"));
  std::vector<DeviceVoltageStateSP> vec;
  vec.reserve(arr_val.elements.size());
  for (const auto &elem : arr_val.elements) {
    vec.push_back(std::get<DeviceVoltageStateSP>(elem));
  }
  auto dvss = std::make_shared<DeviceVoltageStates>(vec);
  pack_dvss(std::move(dvss), out, oc);
}

// ── Mutators ──────────────────────────────────────────────────────────────────

// AddState(this: DeviceVoltageStates, state: DeviceVoltageState) -> ()
void STRUCTDeviceVoltageStatesAddState(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto state = get_opaque<DeviceVoltageState>(params, param_count, "state");
  self->add_state(state);
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Insert(this: DeviceVoltageStates, conn: Connection, location: int) -> ()
void STRUCTDeviceVoltageStatesInsert(const FalconParamEntry *params,
                                      int32_t param_count,
                                      FalconResultSlot *out, int32_t *oc) {
  auto pm      = unpack_params(params, param_count);
  auto self    = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto conn    = get_opaque<Connection>(params, param_count, "conn");
  int64_t loc  = std::get<int64_t>(pm.at("location"));
  self->insert(conn, static_cast<int>(loc));
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// Clear(this: DeviceVoltageStates) -> ()
void STRUCTDeviceVoltageStatesClear(const FalconParamEntry *params,
                                     int32_t param_count,
                                     FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  self->clear();
  out[0] = {}; out[0].tag = FALCON_TYPE_NIL; *oc = 1;
}

// ── Accessors ─────────────────────────────────────────────────────────────────

// FindState(this: DeviceVoltageStates, conn: Connection) -> (DeviceVoltageState state)
void STRUCTDeviceVoltageStatesFindState(const FalconParamEntry *params,
                                         int32_t param_count,
                                         FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  auto state = self->find_state(conn);
  pack_dvs(std::make_shared<DeviceVoltageState>(*state), out, oc);
}

// ToPoint(this: DeviceVoltageStates) -> (point::Point point)
void STRUCTDeviceVoltageStatesToPoint(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto point = self->to_point();
  out[0]                        = {};
  out[0].tag                    = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Point";
  out[0].value.opaque.ptr       = new PointSP(point);
  out[0].value.opaque.deleter   = [](void *p) {
    delete static_cast<PointSP *>(p);
  };
  *oc = 1;
}

// Size(this: DeviceVoltageStates) -> (int size)
void STRUCTDeviceVoltageStatesSize(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  pack_results(FunctionResult{static_cast<int64_t>(self->size())}, out, 16, oc);
}

// Empty(this: DeviceVoltageStates) -> (bool isEmpty)
void STRUCTDeviceVoltageStatesEmpty(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  pack_results(FunctionResult{self->empty()}, out, 16, oc);
}

// Contains(this: DeviceVoltageStates, conn: Connection) -> (bool contains)
void STRUCTDeviceVoltageStatesContains(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto conn = get_opaque<Connection>(params, param_count, "conn");
  pack_results(FunctionResult{self->contains(conn)}, out, 16, oc);
}

// ── Equality ──────────────────────────────────────────────────────────────────

// Equal(this: DeviceVoltageStates, other: DeviceVoltageStates) -> (bool equal)
void STRUCTDeviceVoltageStatesEqual(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto self  = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto other = get_opaque<DeviceVoltageStates>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: DeviceVoltageStates, other: DeviceVoltageStates) -> (bool notequal)
void STRUCTDeviceVoltageStatesNotEqual(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto self  = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto other = get_opaque<DeviceVoltageStates>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON ──────────────────────────────────────────────────────────────────────

// ToJSON(this: DeviceVoltageStates) -> (string json)
void STRUCTDeviceVoltageStatesToJSON(const FalconParamEntry *params,
                                      int32_t param_count, FalconResultSlot *out,
                                      int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  pack_results(FunctionResult{self->to_json_string()}, out, 16, oc);
}

// FromJSON(json: string) -> (DeviceVoltageStates states)
void STRUCTDeviceVoltageStatesFromJSON(const FalconParamEntry *params,
                                        int32_t param_count,
                                        FalconResultSlot *out, int32_t *oc) {
  auto pm   = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto dvss = DeviceVoltageStates::from_json_string<DeviceVoltageStates>(json);
  pack_dvss(std::make_shared<DeviceVoltageStates>(*dvss), out, oc);
}

} // extern "C"
