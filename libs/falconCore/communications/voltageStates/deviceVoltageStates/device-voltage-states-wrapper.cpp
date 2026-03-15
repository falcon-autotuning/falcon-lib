#include "falcon_core/communications/voltage_states/DeviceVoltageState.hpp"
#include "falcon_core/communications/voltage_states/DeviceVoltageStates.hpp"
#include "falcon_core/math/Point.hpp"
#include "falcon_core/physics/device_structures/Connection.hpp"
#include <falcon-typing/FFIHelpers.hpp>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using DeviceVoltageStates =
    falcon_core::communications::voltage_states::DeviceVoltageStates;
using DeviceVoltageStatesSP = std::shared_ptr<DeviceVoltageStates>;
using DeviceVoltageState =
    falcon_core::communications::voltage_states::DeviceVoltageState;
using DeviceVoltageStateSP = std::shared_ptr<DeviceVoltageState>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;
using Point = falcon_core::math::Point;
using PointSP = std::shared_ptr<Point>;

// ── helpers
// ───────────────────────────────────────────────────────────────────

static void pack_dvss(DeviceVoltageStatesSP dvss, FalconResultSlot *out,
                      int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceVoltageStates";
  out[0].value.opaque.ptr = new DeviceVoltageStatesSP(std::move(dvss));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<DeviceVoltageStatesSP *>(p);
  };
  *oc = 1;
}

static void pack_dvs(DeviceVoltageStateSP dvs, FalconResultSlot *out,
                     int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "DeviceVoltageState";
  out[0].value.opaque.ptr = new DeviceVoltageStateSP(std::move(dvs));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<DeviceVoltageStateSP *>(p);
  };
  *oc = 1;
}

// ---------------------------------------------------------------------------
// get_array_from_params
//
// Recover the shared_ptr<ArrayValue> from a FalconParamEntry for a parameter
// that holds an Array<T> value.
//
// The engine's pack_params path for a native StructInstance (is_native==true)
// does:
//   ptr = new shared_ptr<void>( val->native_handle.value() )
//   type_name = val->type_name   (= "Array")
//
// So ptr is a heap-allocated shared_ptr<void>*, and .get() points at the
// ArrayValue.  We recover it via static_pointer_cast<ArrayValue>.
// ---------------------------------------------------------------------------
static std::shared_ptr<ArrayValue>
get_array_from_params(const FalconParamEntry *entries, int32_t count,
                      const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(entries[i].key, key) != 0)
      continue;

    const FalconParamEntry &e = entries[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(
          std::string("STRUCTDeviceVoltageStatesNew: parameter '") + key +
          "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    }

    std::string tn = e.value.opaque.type_name ? e.value.opaque.type_name : "";

    // Normal path: engine packed a native StructInstance ("Array") as
    //   ptr = new shared_ptr<void>* aliasing the ArrayValue
    if (tn == "Array") {
      auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
      return std::static_pointer_cast<ArrayValue>(sv);
    }

    // Legacy path: someone passed a bare ArrayValue opaque directly
    if (tn == "ArrayValue") {
      return *static_cast<std::shared_ptr<ArrayValue> *>(e.value.opaque.ptr);
    }

    throw std::runtime_error(
        std::string("STRUCTDeviceVoltageStatesNew: parameter '") + key +
        "' has unexpected opaque type_name='" + tn + "'");
  }
  throw std::runtime_error(
      std::string("STRUCTDeviceVoltageStatesNew: parameter '") + key +
      "' not found");
}

extern "C" {

// ── Constructor
// ───────────────────────────────────────────────────────────────

// New(ports: Array<DeviceVoltageState>) -> (DeviceVoltageStates dstates)
void STRUCTDeviceVoltageStatesNew(const FalconParamEntry *params,
                                  int32_t param_count, FalconResultSlot *out,
                                  int32_t *oc) {
  auto arr_val = get_array_from_params(params, param_count, "dstates");

  std::vector<DeviceVoltageStateSP> vec;
  vec.reserve(arr_val->elements.size());

  for (const auto &elem : arr_val->elements) {
    // Each element was packed by instrumentport-wrapper.cpp as a StructInstance
    // with native_handle holding the DeviceVoltageStateSP.
    // However unpack_results on the PushBack call stored it as a
    // StructInstance directly in the RuntimeValue — so we get a
    // shared_ptr<StructInstance> here, not a shared_ptr<void>.
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem)) {
      throw std::runtime_error(
          "STRUCTDeviceVoltageStatesNew: array element is not a "
          "StructInstance (variant index=" +
          std::to_string(elem.index()) + ")");
    }
    auto inst = std::get<std::shared_ptr<StructInstance>>(elem);
    if (!inst) {
      throw std::runtime_error(
          "STRUCTDeviceVoltageStatesNew: StructInstance element is null");
    }
    if (!inst->native_handle.has_value()) {
      throw std::runtime_error(
          "STRUCTDeviceVoltageStatesNew: DeviceVoltageState StructInstance has "
          "no native_handle "
          "(type=" +
          inst->type_name + ")");
    }
    auto port = std::static_pointer_cast<DeviceVoltageState>(
        inst->native_handle.value());
    if (!port) {
      throw std::runtime_error(
          "STRUCTDeviceVoltageStatesNew: "
          "static_pointer_cast<DeviceVoltageState> returned null");
    }
    vec.push_back(std::move(port));
  }

  pack_dvss(
      std::make_shared<DeviceVoltageStates>(
          std::make_shared<falcon_core::generic::List<DeviceVoltageState>>(
              vec)),
      out, oc);
}

// ── Mutators
// ──────────────────────────────────────────────────────────────────

// AddState(this: DeviceVoltageStates, state: DeviceVoltageState) -> ()
void STRUCTDeviceVoltageStatesAddState(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto state = get_opaque<DeviceVoltageState>(params, param_count, "dstate");
  self->add_state(state);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Insert(this: DeviceVoltageStates, conn: Connection, location: int) -> ()
void STRUCTDeviceVoltageStatesInsert(const FalconParamEntry *params,
                                     int32_t param_count, FalconResultSlot *out,
                                     int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto conn = get_opaque<DeviceVoltageState>(params, param_count, "dstate");
  int64_t loc = std::get<int64_t>(pm.at("location"));
  auto it = self->begin() + static_cast<int>(loc);
  std::vector<std::shared_ptr<DeviceVoltageState>> tmp{conn};
  self->insert(it, tmp.begin(), tmp.end());
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// Clear(this: DeviceVoltageStates) -> ()
void STRUCTDeviceVoltageStatesClear(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  self->clear();
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── Accessors
// ─────────────────────────────────────────────────────────────────

// FindState(this: DeviceVoltageStates, conn: Connection) -> (DeviceVoltageState
// state)
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
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto point = self->to_point();
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Point";
  out[0].value.opaque.ptr = new PointSP(point);
  out[0].value.opaque.deleter = [](void *p) {
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
  auto conn = get_opaque<DeviceVoltageState>(params, param_count, "dstate");
  pack_results(FunctionResult{self->contains(conn)}, out, 16, oc);
}

// ── Equality
// ──────────────────────────────────────────────────────────────────

// Equal(this: DeviceVoltageStates, other: DeviceVoltageStates) -> (bool equal)
void STRUCTDeviceVoltageStatesEqual(const FalconParamEntry *params,
                                    int32_t param_count, FalconResultSlot *out,
                                    int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto other = get_opaque<DeviceVoltageStates>(params, param_count, "other");
  pack_results(FunctionResult{*self == *other}, out, 16, oc);
}

// NotEqual(this: DeviceVoltageStates, other: DeviceVoltageStates) -> (bool
// notequal)
void STRUCTDeviceVoltageStatesNotEqual(const FalconParamEntry *params,
                                       int32_t param_count,
                                       FalconResultSlot *out, int32_t *oc) {
  auto self = get_opaque<DeviceVoltageStates>(params, param_count, "this");
  auto other = get_opaque<DeviceVoltageStates>(params, param_count, "other");
  pack_results(FunctionResult{*self != *other}, out, 16, oc);
}

// ── JSON
// ──────────────────────────────────────────────────────────────────────

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
  auto pm = unpack_params(params, param_count);
  auto json = std::get<std::string>(pm.at("json"));
  auto dvss = DeviceVoltageStates::from_json_string<DeviceVoltageStates>(json);
  pack_dvss(std::make_shared<DeviceVoltageStates>(*dvss), out, oc);
}

} // extern "C"
