// qarray-wrapper.cpp — FFI binding for the charge-tuning demo hub.fal
//
// Implements four free routines (not struct methods):
//   CollectCurrentDeviceState() -> (DeviceVoltageStates dstates)
//   CollectCurrentPlungerGates() -> (Array<Connection> gates)
//   Ramp(Map<Connection,Quantity> stop) -> ()
//   AnalyzeBlips(Map<Connection,Quantity> start,
//                Map<Connection,Quantity> stop,
//                int resolution)
//       -> (int num_blips, Array<DeviceVoltageStates> locations)
//
// Naming convention for free routines: the function symbol is just the
// routine name (e.g. FUNCCollectCurrentDeviceState), matching the FAL
// ffimport convention for top-level routines.

#include <algorithm>
#include <cmath>
#include <falcon-typing/FFIHelpers.hpp>
#include <falcon_core/communications/voltage_states/DeviceVoltageState.hpp>
#include <falcon_core/communications/voltage_states/DeviceVoltageStates.hpp>
#include <falcon_core/generic/List.hpp>
#include <falcon_core/math/Quantity.hpp>
#include <falcon_core/physics/device_structures/Connection.hpp>
#include <memory>
#include <qarrayDevice/Device.hpp>
#include <stdexcept>
#include <string>
#include <vector>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

using DeviceVoltageState =
    falcon_core::communications::voltage_states::DeviceVoltageState;
using DeviceVoltageStateSP = std::shared_ptr<DeviceVoltageState>;
using DeviceVoltageStates =
    falcon_core::communications::voltage_states::DeviceVoltageStates;
using DeviceVoltageStatesSP = std::shared_ptr<DeviceVoltageStates>;
using Connection = falcon_core::physics::device_structures::Connection;
using ConnectionSP = std::shared_ptr<Connection>;
using Quantity = falcon_core::math::Quantity;
using QuantitySP = std::shared_ptr<Quantity>;
using SymbolUnit = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP = std::shared_ptr<SymbolUnit>;
using QDevice = falcon::qarray::Device;

// ── Module-level singleton device
// ─────────────────────────────────────────────
//
// The Device embeds CPython via pybind11.  Only one interpreter may be active
// per process, and Device construction is expensive, so we keep a single
// lazily-initialised instance.  The config path is baked in here; for a
// production binding it could be passed as a module-init parameter.
//
// NOTE: this is intentionally a raw pointer so it is never destroyed (avoids
// pybind11 / Python atexit ordering issues).

static QDevice *g_device = nullptr;

static QDevice &device() {
  if (g_device == nullptr) {
    // Default config path — override by recompiling with -DQARRAY_CONFIG=...
#ifndef QARRAY_CONFIG
#define QARRAY_CONFIG "/opt/qarray/device.yaml"
#endif
    g_device = new QDevice(QARRAY_CONFIG);
  }
  return *g_device;
}

// ── pack helpers
// ──────────────────────────────────────────────────────────────

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

// Pack an Array<Connection> as a DSL-compatible Array opaque.
// Each Connection is a StructInstance with native_handle set to its SP.
static void pack_connection_array(std::vector<ConnectionSP> conns,
                                  FalconResultSlot *out, int32_t *oc) {
  auto arr = std::make_shared<ArrayValue>("Connection");
  arr->elements.reserve(conns.size());
  for (auto &conn : conns) {
    auto inst = std::make_shared<StructInstance>("Connection");
    inst->native_handle = std::static_pointer_cast<void>(conn);
    arr->elements.push_back(std::move(inst));
  }
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// Pack an Array<DeviceVoltageStates> for AnalyzeBlips locations output.
static void pack_dvss_array(std::vector<DeviceVoltageStatesSP> items,
                            FalconResultSlot *out, int32_t *oc) {
  auto arr = std::make_shared<ArrayValue>("DeviceVoltageStates");
  arr->elements.reserve(items.size());
  for (auto &dvss : items) {
    auto inst = std::make_shared<StructInstance>("DeviceVoltageStates");
    inst->native_handle = std::static_pointer_cast<void>(dvss);
    arr->elements.push_back(std::move(inst));
  }
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Array";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(arr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}

// ── DSL Map<Connection,Quantity> → std::map<string,double> ───────────────────
//
// The DSL Map<K,V> is a pure-FAL struct (no native_handle of its own).
// It arrives as an OPAQUE whose ptr is a shared_ptr<void>* aliasing a
// shared_ptr<StructInstance> (type_name="Map").
// Its fields are:
//   keys_   : Array<Connection>   — elements are
//   StructInstance{native_handle=ConnectionSP} values_ : Array<Quantity>     —
//   elements are StructInstance{native_handle=QuantitySP}
//
// We retrieve the StructInstance, then walk its keys_/values_ Array fields.

// Helper: retrieve the ArrayValue for a named field inside a StructInstance.
static std::shared_ptr<ArrayValue>
field_array(const std::shared_ptr<StructInstance> &inst, const char *field) {
  auto it = inst->fields->find(field);
  if (it == inst->fields->end()) {
    throw std::runtime_error(
        std::string("qarray-wrapper: Map StructInstance missing field '") +
        field + "'");
  }
  const RuntimeValue &fv = it->second;
  if (!std::holds_alternative<std::shared_ptr<StructInstance>>(fv)) {
    throw std::runtime_error(std::string("qarray-wrapper: Map field '") +
                             field + "' is not a StructInstance");
  }
  auto arr_inst = std::get<std::shared_ptr<StructInstance>>(fv);
  if (!arr_inst || !arr_inst->native_handle.has_value()) {
    throw std::runtime_error(std::string("qarray-wrapper: Map field '") +
                             field + "' StructInstance has no native_handle");
  }
  return std::static_pointer_cast<ArrayValue>(arr_inst->native_handle.value());
}

// Helper: get a DSL Map StructInstance from the param list by name.
// The Map arrives as OPAQUE{type_name="Map", ptr=shared_ptr<void>* aliasing
// shared_ptr<StructInstance>}.
static std::shared_ptr<StructInstance>
get_map_param(const FalconParamEntry *params, int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(params[i].key, key) != 0) {
      continue;
    }
    const FalconParamEntry &e = params[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(
          std::string("qarray-wrapper: parameter '") + key +
          "' is not OPAQUE (tag=" + std::to_string(e.tag) + ")");
    }
    // ptr is a heap-allocated shared_ptr<void>* whose managed object is the
    // StructInstance (same convention as native StructInstances passed by the
    // engine: ptr = new shared_ptr<void>(native_handle)).
    auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
    return std::static_pointer_cast<StructInstance>(sv);
  }
  throw std::runtime_error(std::string("qarray-wrapper: parameter '") + key +
                           "' not found");
}

// Convert a DSL Map<Connection,Quantity> to std::map<string,double> in Volts.
//
// For each parallel entry i:
//   key   = Connection StructInstance  → conn->name()
//   value = Quantity   StructInstance  → convert to Volt, then q->value()
static std::map<std::string, double>
dsl_map_to_volt_map(const std::shared_ptr<StructInstance> &map_inst) {
  auto keys_arr = field_array(map_inst, "keys_");
  auto vals_arr = field_array(map_inst, "values_");

  if (keys_arr->elements.size() != vals_arr->elements.size()) {
    throw std::runtime_error(
        "qarray-wrapper: Map<Connection,Quantity> keys_ and values_ "
        "have different sizes");
  }

  auto volt_unit = SymbolUnit::Volt();
  std::map<std::string, double> result;

  for (size_t i = 0; i < keys_arr->elements.size(); ++i) {
    // ── key: Connection StructInstance with native_handle
    const RuntimeValue &kv = keys_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(kv)) {
      throw std::runtime_error(
          "qarray-wrapper: Map key element is not a StructInstance");
    }
    auto k_inst = std::get<std::shared_ptr<StructInstance>>(kv);
    if (!k_inst || !k_inst->native_handle.has_value()) {
      throw std::runtime_error(
          "qarray-wrapper: Map key StructInstance has no native_handle");
    }
    auto conn =
        std::static_pointer_cast<Connection>(k_inst->native_handle.value());

    // ── value: Quantity StructInstance with native_handle
    const RuntimeValue &vv = vals_arr->elements[i];
    if (!std::holds_alternative<std::shared_ptr<StructInstance>>(vv)) {
      throw std::runtime_error(
          "qarray-wrapper: Map value element is not a StructInstance");
    }
    auto v_inst = std::get<std::shared_ptr<StructInstance>>(vv);
    if (!v_inst || !v_inst->native_handle.has_value()) {
      throw std::runtime_error(
          "qarray-wrapper: Map value StructInstance has no native_handle");
    }
    auto qty =
        std::static_pointer_cast<Quantity>(v_inst->native_handle.value());

    // Convert Quantity to Volt and extract scalar value
    auto qty_copy = std::make_shared<Quantity>(*qty);
    qty_copy->convert_to(volt_unit);
    double volts = qty_copy->value();

    result[conn->name()] = volts;
  }
  return result;
}

// ── build_dvss_from_volt_map
// ──────────────────────────────────────────────────
//
// Given a std::map<string,double> of gate→voltage-in-Volts, construct a
// DeviceVoltageStates where each entry is:
//   DeviceVoltageState( Connection::PlungerGate(name), voltage, Volt )
static DeviceVoltageStatesSP
build_dvss_from_volt_map(const std::map<std::string, double> &volt_map) {
  auto volt_unit = SymbolUnit::Volt();
  std::vector<DeviceVoltageStateSP> vec;
  vec.reserve(volt_map.size());
  for (const auto &kv : volt_map) {
    auto conn = Connection::PlungerGate(kv.first);
    vec.push_back(
        std::make_shared<DeviceVoltageState>(conn, kv.second, volt_unit));
  }
  return std::make_shared<DeviceVoltageStates>(
      std::make_shared<falcon_core::generic::List<DeviceVoltageState>>(vec));
}

// ── peak detection
// ────────────────────────────────────────────────────────────
//
// Simple local-maximum detector on a 1-D signal with significance threshold.
//
// A sample at index i is considered a peak when:
//   signal[i] > signal[i-1]  AND  signal[i] > signal[i+1]   (local max)
//   AND  signal[i] > mean + threshold_sigma * stddev          (significant)
//
// Returns the indices of detected peaks.
static std::vector<int> detect_peaks(const std::vector<double> &signal,
                                     double threshold_sigma = 2.0) {
  const int n = static_cast<int>(signal.size());
  if (n < 3) {
    return {};
  }

  // Compute mean and stddev
  double sum = 0.0;
  for (double v : signal) {
    sum += v;
  }
  double mean = sum / n;

  double sq_sum = 0.0;
  for (double v : signal) {
    sq_sum += (v - mean) * (v - mean);
  }
  double stddev = std::sqrt(sq_sum / n);

  double threshold = mean + (threshold_sigma * stddev);

  std::vector<int> peaks;
  for (int i = 1; i < n - 1; ++i) {
    if (signal[i] > signal[i - 1] && signal[i] > signal[i + 1] &&
        signal[i] > threshold) {
      peaks.push_back(i);
    }
  }
  return peaks;
}

// ─────────────────────────────────────────────────────────────────────────────
// Exported FFI functions
// ─────────────────────────────────────────────────────────────────────────────

extern "C" {

// ── CollectCurrentDeviceState() -> (DeviceVoltageStates dstates) ─────────────
//
// 1. Call device().voltages()  → std::map<string,double>
// 2. For each gate: create Connection::PlungerGate(name)
//                   create DeviceVoltageState(conn, voltage_V, Volt)
// 3. Wrap in DeviceVoltageStates and return.
void CollectCurrentDeviceState(const FalconParamEntry * /*params*/,
                               int32_t /*param_count*/, FalconResultSlot *out,
                               int32_t *oc) {
  const auto &volt_map = device().voltages();
  auto dvss = build_dvss_from_volt_map(volt_map);
  pack_dvss(std::move(dvss), out, oc);
}

// ── CollectCurrentPlungerGates() -> (Array<Connection> gates) ────────────────
//
// 1. Call device().gate_names() → vector<string>
// 2. For each name: create Connection::PlungerGate(name)
// 3. Return as a DSL Array<Connection>.
void CollectCurrentPlungerGates(const FalconParamEntry * /*params*/,
                                int32_t /*param_count*/, FalconResultSlot *out,
                                int32_t *oc) {
  const auto &names = device().gate_names();
  std::vector<ConnectionSP> conns;
  conns.reserve(names.size());
  for (const auto &name : names) {
    conns.push_back(Connection::PlungerGate(name));
  }
  pack_connection_array(std::move(conns), out, oc);
}

// ── Ramp(stop: Map<Connection,Quantity>) -> ()
// ────────────────────────────────
//
// Convert the DSL Map<Connection,Quantity> stop to std::map<string,double>
// in Volts, then call device().scan_ray(current_voltages, stop, resolution=4).
// The scan result is discarded — we only want the physical ramp side-effect.
void Ramp(const FalconParamEntry *params, int32_t param_count,
          FalconResultSlot *out, int32_t *oc) {
  auto stop_inst = get_map_param(params, param_count, "stop");
  auto stop_map = dsl_map_to_volt_map(stop_inst);

  // Use current device voltages as the start point of the ramp
  auto start_map = device().voltages();

  // Resolution is fixed at 4 for Ramp
  constexpr int ramp_resolution = 4;

  // scan_ray performs the physical sweep from start to stop
  device().scan_ray(start_map, stop_map, ramp_resolution);

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── AnalyzeBlips(start, stop, resolution)
//       -> (int num_blips, Array<DeviceVoltageStates> locations) ──────────────
//
// 1. Convert DSL Maps to std::map<string,double> in Volts.
// 2. Call device().scan_ray(start, stop, resolution).
// 3. Take ScanResultRay::differentiated_signal (flat [resolution * n_sensors]).
//    We use the first sensor channel (column 0 of row-major layout).
// 4. Run peak detection on that channel.
// 5. For each peak index i:
//    a. Read ScanResultRay::trajectory[i * n_gates + gate_idx] to get the
//       applied voltage on each gate at position i along the sweep.
//       Gate ordering in trajectory matches the key order of `start`.
//    b. Build a DeviceVoltageStates from those voltages.
// 6. Return (num_peaks, Array<DeviceVoltageStates>).
//
// Multi-return convention: the FAL runtime expects multiple FalconResultSlots.
// slot[0] = int num_blips
// slot[1] = Array<DeviceVoltageStates> locations
void AnalyzeBlips(const FalconParamEntry *params, int32_t param_count,
                  FalconResultSlot *out, int32_t *oc) {
  // 1. Decode inputs
  auto start_inst = get_map_param(params, param_count, "start");
  auto stop_inst = get_map_param(params, param_count, "stop");
  auto pm = unpack_params(params, param_count);
  int resolution = static_cast<int>(std::get<int64_t>(pm.at("resolution")));

  auto start_map = dsl_map_to_volt_map(start_inst);
  auto stop_map = dsl_map_to_volt_map(stop_inst);

  // 2. Run the scan
  auto result = device().scan_ray(start_map, stop_map, resolution);

  // 3. Extract the differentiated signal for the first sensor channel.
  //    Shape: [resolution * n_sensors], row-major → column 0 per row.
  std::vector<double> signal;
  signal.reserve(static_cast<size_t>(resolution));

  if (result.has_sensor && !result.differentiated_signal.empty()) {
    // n_sensors can be inferred from the shape vector if present, otherwise
    // from total size / resolution.
    int n_sensors = 1;
    if (!result.differentiated_signal_shape.empty() &&
        result.differentiated_signal_shape.size() >= 2) {
      n_sensors = result.differentiated_signal_shape[1];
    } else if (result.differentiated_signal.size() > 0 && resolution > 0) {
      n_sensors =
          static_cast<int>(result.differentiated_signal.size()) / resolution;
      n_sensors = std::max(n_sensors, 1);
    }
    for (int i = 0; i < resolution; ++i) {
      auto idx = static_cast<size_t>(i * n_sensors); // column 0 of row i
      if (idx < result.differentiated_signal.size()) {
        signal.push_back(result.differentiated_signal[idx]);
      } else {
        signal.push_back(0.0);
      }
    }
  } else {
    // No sensor data — no blips possible
    signal.assign(static_cast<size_t>(resolution), 0.0);
  }

  // 4. Detect peaks (significant local maxima)
  auto peak_indices = detect_peaks(signal);

  // 5. For each peak, reconstruct the voltage state at that sweep position.
  //    trajectory shape: [resolution * n_gates], row-major.
  //    Gate order matches the iteration order of start_map (std::map = sorted).
  std::vector<std::string> gate_order;
  gate_order.reserve(start_map.size());
  for (const auto &kv : start_map) {
    gate_order.push_back(kv.first);
  }

  const int n_gates = static_cast<int>(gate_order.size());
  auto volt_unit = SymbolUnit::Volt();

  std::vector<DeviceVoltageStatesSP> locations;
  locations.reserve(peak_indices.size());

  for (int peak_idx : peak_indices) {
    std::vector<DeviceVoltageStateSP> dvs_vec;
    dvs_vec.reserve(static_cast<size_t>(n_gates));

    for (int g = 0; g < n_gates; ++g) {
      double v = 0.0;
      auto traj_idx = static_cast<size_t>((peak_idx * n_gates) + g);
      if (!result.trajectory.empty() && traj_idx < result.trajectory.size()) {
        v = result.trajectory[traj_idx];
      }
      auto conn = Connection::PlungerGate(gate_order[static_cast<size_t>(g)]);
      dvs_vec.push_back(
          std::make_shared<DeviceVoltageState>(conn, v, volt_unit));
    }

    locations.push_back(std::make_shared<DeviceVoltageStates>(
        std::make_shared<falcon_core::generic::List<DeviceVoltageState>>(
            dvs_vec)));
  }

  // 6. Pack two return values
  int num_blips = static_cast<int>(peak_indices.size());

  // slot[0]: int num_blips
  out[0] = {};
  out[0].tag = FALCON_TYPE_INT;
  out[0].value.int_val = static_cast<int64_t>(num_blips);

  // slot[1]: Array<DeviceVoltageStates> locations
  // We call the pack helper but need to write to out[1].
  {
    FalconResultSlot tmp[1];
    int32_t tmp_oc = 0;
    pack_dvss_array(std::move(locations), tmp, &tmp_oc);
    out[1] = tmp[0];
  }

  *oc = 2;
}

} // extern "C"
