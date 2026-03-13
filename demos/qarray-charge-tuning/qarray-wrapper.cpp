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
//   EndState(Connection direction, Quantity spacing, FArray virtualization,
//            Connections virtualNames) -> (Point endstate)
//
// Naming convention for free routines: the function symbol is just the
// routine name (e.g. FUNCCollectCurrentDeviceState), matching the FAL
// ffimport convention for top-level routines.

#include <algorithm>
#include <cmath>
#include <dlfcn.h>
#include <falcon-typing/FFIHelpers.hpp>
#include <falcon_core/communications/voltage_states/DeviceVoltageState.hpp>
#include <falcon_core/communications/voltage_states/DeviceVoltageStates.hpp>
#include <falcon_core/generic/FArray.hpp>
#include <falcon_core/generic/List.hpp>
#include <falcon_core/math/Point.hpp>
#include <falcon_core/math/Quantity.hpp>
#include <falcon_core/physics/device_structures/Connection.hpp>
#include <matplot/matplot.h>
#include <memory>
#include <plplot/plplot.h>
#include <plplot/plplotP.h>
#include <plplot/plstream.h>
#include <qarrayDevice/Device.hpp>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef PI
#undef PI
#endif
#include <pybind11/embed.h>
#include <xtensor/xadapt.hpp>
#include <yaml-cpp/yaml.h>
namespace {
static bool python_initialized = false;
static void ensure_python_interpreter() {
  if (!python_initialized) {
    dlopen("/opt/falcon/lib/libpython3.12.so", RTLD_NOW | RTLD_GLOBAL);
    static pybind11::scoped_interpreter guard{};
    python_initialized = true;
  }
}
} // namespace

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
using Point = falcon_core::math::Point;
using PointSP = std::shared_ptr<Point>;
using QDevice = falcon::qarray::Device;

// ── Module-level singleton device
// ─────────────────────────────────────────────────────
//
// The Device embeds CPython via pybind11.  Only one interpreter may be active
// per process, and Device construction is expensive, so we keep a single
// lazily-initialised instance.  The config path is baked in here; for a
// production binding it could be passed as a module-init parameter.
//
// NOTE: this is intentionally a raw pointer so it is never destroyed (avoids
// pybind11 / Python atexit ordering issues).

static QDevice *g_device = nullptr;
static std::string current_config_path;

// Always initialize at startup using env or default
static void initialize_device() {
  ensure_python_interpreter();
  const char *env_path = std::getenv("QARRAY_CONFIG_PATH");
#ifndef QARRAY_CONFIG
#define QARRAY_CONFIG "/opt/qarray/device.yaml"
#endif
  std::string config_path = env_path ? env_path : QARRAY_CONFIG;
  if (g_device != nullptr && current_config_path == config_path) {
    // Already initialized with this config, do nothing
    return;
  }
  if (g_device != nullptr) {
    delete g_device;
    g_device = nullptr;
  }
  g_device = new QDevice(config_path);
  current_config_path = config_path;
}

static QDevice &device() {
  if (g_device == nullptr) {
    initialize_device();
  }
  return *g_device;
}

// Restart the device singleton with a new config path
void restart_device(const std::string &config_path) {
  spdlog::info("The config_path is " + config_path);
  ensure_python_interpreter();
  if (g_device != nullptr && current_config_path == config_path) {
    // Already initialized with this config, do nothing
    return;
  }
  if (g_device != nullptr) {
    delete g_device;
    g_device = nullptr;
  }
  g_device = new QDevice(config_path);
  current_config_path = config_path;
}

// Optionally, expose this via extern "C" for FFI or module API
extern "C" void RestartDevice(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string config_path = std::get<std::string>(pm.at("configPath"));
  restart_device(config_path);
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

// ── pack helpers
// ──────────────────────────────────────────────────────

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

// Pack a Point as a DSL-compatible opaque.
static void pack_point(PointSP point, FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "Point";
  out[0].value.opaque.ptr = new PointSP(std::move(point));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<PointSP *>(p);
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

// Helper: get a DSL opaque parameter (Connection, Point, etc.) by name.
template <typename T>
static std::shared_ptr<T> get_opaque_param(const FalconParamEntry *params,
                                           int32_t count, const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(params[i].key, key) != 0) {
      continue;
    }
    const FalconParamEntry &e = params[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(std::string("qarray-wrapper: parameter '") +
                               key + "' is not OPAQUE");
    }
    auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
    return std::static_pointer_cast<T>(sv);
  }
  throw std::runtime_error(std::string("qarray-wrapper: parameter '") + key +
                           "' not found");
}

// Helper: get a DSL Array parameter by name.
static std::shared_ptr<ArrayValue>
get_array_param(const FalconParamEntry *params, int32_t count,
                const char *key) {
  for (int32_t i = 0; i < count; ++i) {
    if (std::strcmp(params[i].key, key) != 0) {
      continue;
    }
    const FalconParamEntry &e = params[i];
    if (e.tag != FALCON_TYPE_OPAQUE) {
      throw std::runtime_error(std::string("qarray-wrapper: parameter '") +
                               key + "' is not OPAQUE");
    }
    auto sv = *static_cast<std::shared_ptr<void> *>(e.value.opaque.ptr);
    return std::static_pointer_cast<ArrayValue>(sv);
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
// ──────────────────────────────────────────────────────
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
// ─────────────────────────────────────��───────────────────────────────────────

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
// ────────────────────────────────────
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
  auto start_inst = get_map_param(params, param_count, "begin");
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

// ── EndState(direction, spacing, virtualization, virtualNames)
//       -> (Point endstate)
//       ────────────────────────────────────────────────────
//
// Algorithm:
// 1. Create a unit vector where the index corresponding to the `direction`
//    Connection (as found in virtualNames) is 1.0, all others are 0.0.
// 2. Multiply this unit vector by the inverse of the virtualization matrix.
// 3. Scale the result by the spacing Quantity.
// 4. Get the current device state (DeviceVoltageStates → Point).
// 5. Add the calculated vector to the current state for matching Connections.
// 6. Return the resulting Point.
void EndState(const FalconParamEntry *params, int32_t param_count,
              FalconResultSlot *out, int32_t *oc) {
  try {
    // Unpack parameters
    auto direction =
        get_opaque_param<Connection>(params, param_count, "direction");
    auto spacing = get_opaque_param<Quantity>(params, param_count, "spacing");
    auto virtualization_arr =
        get_array_param(params, param_count, "virtualization");
    auto virtual_names_arr =
        get_array_param(params, param_count, "virtualNames");

    // Extract virtual names as a vector of Connections
    std::vector<ConnectionSP> virtual_names;
    virtual_names.reserve(virtual_names_arr->elements.size());

    for (const auto &elem : virtual_names_arr->elements) {
      if (!std::holds_alternative<std::shared_ptr<StructInstance>>(elem)) {
        throw std::runtime_error(
            "EndState: virtualNames element is not a StructInstance");
      }
      auto conn_inst = std::get<std::shared_ptr<StructInstance>>(elem);
      if (!conn_inst || !conn_inst->native_handle.has_value()) {
        throw std::runtime_error(
            "EndState: virtualNames element has no native_handle");
      }
      virtual_names.push_back(std::static_pointer_cast<Connection>(
          conn_inst->native_handle.value()));
    }

    const int n_gates = static_cast<int>(virtual_names.size());

    // Step 1: Create a unit vector for the direction
    std::vector<double> unit_vector(n_gates, 0.0);
    bool direction_found = false;

    for (int i = 0; i < n_gates; ++i) {
      if (virtual_names[i]->name() == direction->name() &&
          virtual_names[i]->type() == direction->type()) {
        unit_vector[i] = 1.0;
        direction_found = true;
        break;
      }
    }

    if (!direction_found) {
      throw std::runtime_error(
          "EndState: direction Connection not found in virtualNames");
    }

    // Step 2: Extract the virtualization matrix from the FArray
    // FArray is a 2D array of floats in row-major order: [n_gates x n_gates]
    std::vector<double> virt_matrix;
    virt_matrix.reserve(virtualization_arr->elements.size());

    for (const auto &elem : virtualization_arr->elements) {
      double value = 0.0;
      if (std::holds_alternative<int64_t>(elem)) {
        value = static_cast<double>(std::get<int64_t>(elem));
      } else if (std::holds_alternative<double>(elem)) {
        value = std::get<double>(elem);
      } else {
        throw std::runtime_error(
            "EndState: virtualization matrix element is not numeric");
      }
      virt_matrix.push_back(value);
    }

    if (virt_matrix.size() != static_cast<size_t>(n_gates * n_gates)) {
      throw std::runtime_error(
          "EndState: virtualization matrix size mismatch (expected " +
          std::to_string(n_gates * n_gates) + ", got " +
          std::to_string(virt_matrix.size()) + ")");
    }

    // Step 3: Compute the inverse of the virtualization matrix (using Gaussian
    // elimination)
    std::vector<double> virt_inv = virt_matrix;
    std::vector<double> identity(n_gates * n_gates, 0.0);
    for (int i = 0; i < n_gates; ++i) {
      identity[i * n_gates + i] = 1.0;
    }

    // Forward elimination
    for (int i = 0; i < n_gates; ++i) {
      // Find pivot
      int pivot_row = i;
      for (int j = i + 1; j < n_gates; ++j) {
        if (std::abs(virt_inv[j * n_gates + i]) >
            std::abs(virt_inv[pivot_row * n_gates + i])) {
          pivot_row = j;
        }
      }

      // Swap rows
      if (pivot_row != i) {
        for (int j = 0; j < n_gates; ++j) {
          std::swap(virt_inv[i * n_gates + j],
                    virt_inv[pivot_row * n_gates + j]);
          std::swap(identity[i * n_gates + j],
                    identity[pivot_row * n_gates + j]);
        }
      }

      // Check for singular matrix
      if (std::abs(virt_inv[i * n_gates + i]) < 1e-10) {
        throw std::runtime_error(
            "EndState: virtualization matrix is singular or ill-conditioned");
      }

      // Eliminate column
      double pivot = virt_inv[i * n_gates + i];
      for (int j = 0; j < n_gates; ++j) {
        virt_inv[i * n_gates + j] /= pivot;
        identity[i * n_gates + j] /= pivot;
      }

      for (int j = 0; j < n_gates; ++j) {
        if (i != j) {
          double factor = virt_inv[j * n_gates + i];
          for (int k = 0; k < n_gates; ++k) {
            virt_inv[j * n_gates + k] -= factor * virt_inv[i * n_gates + k];
            identity[j * n_gates + k] -= factor * identity[i * n_gates + k];
          }
        }
      }
    }

    // Step 4: Multiply inverse matrix by unit vector
    // result = inv(virtualization) * unit_vector
    std::vector<double> result_vector(n_gates, 0.0);
    for (int i = 0; i < n_gates; ++i) {
      for (int j = 0; j < n_gates; ++j) {
        result_vector[i] += identity[i * n_gates + j] * unit_vector[j];
      }
    }

    // Step 5: Scale by spacing Quantity
    auto volt_unit = SymbolUnit::Volt();
    auto spacing_copy = std::make_shared<Quantity>(*spacing);
    spacing_copy->convert_to(volt_unit);
    double spacing_value = spacing_copy->value();

    for (double &v : result_vector) {
      v *= spacing_value;
    }

    // Step 6: Get current device state and create adjustment Point
    const auto &current_voltages = device().voltages();
    auto adjustment = std::make_shared<Point>();

    for (int i = 0; i < n_gates; ++i) {
      auto conn = virtual_names[i];
      auto qty = std::make_shared<Quantity>(result_vector[i], volt_unit);
      adjustment->insert(conn, qty);
    }

    // Step 7: Get current state as Point
    auto current_state = build_dvss_from_volt_map(current_voltages);
    auto current_point = current_state->to_point();

    // Step 8: Add adjustment to current state
    // Point supports addition operator
    auto final_point = current_point->operator+(adjustment);

    // Pack and return
    pack_point(std::move(final_point), out, oc);

  } catch (const std::exception &e) {
    // Error handling: set error result
    out[0] = {};
    out[0].tag = FALCON_TYPE_NIL;
    *oc = 1;
    throw;
  }
}

void MakeStabilityDiagram(const FalconParamEntry *params, int32_t param_count,
                          FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string output_filepath = std::get<std::string>(pm.at("outputFilepath"));
  auto gate_names = device().gate_names();
  if (gate_names.size() < 2) {
    out[0] = {};
    out[0].tag = FALCON_TYPE_NIL;
    *oc = 1;
    return;
  }
  std::string x_gate = gate_names[0];
  std::string y_gate = gate_names[1];
  int resolution = 100;
  auto result =
      device().scan_2d(x_gate, y_gate, {-1.0, 1.0}, {-1.0, 1.0}, resolution);

  std::vector<double> data;
  if (result.has_sensor && !result.differentiated_signal.empty()) {
    int n_sensors = 1;
    if (!result.differentiated_signal_shape.empty() &&
        result.differentiated_signal_shape.size() >= 2) {
      n_sensors = result.differentiated_signal_shape[1];
    }
    for (int i = 0; i < resolution * resolution; ++i) {
      int idx = i * n_sensors;
      if (idx < result.differentiated_signal.size()) {
        data.push_back(result.differentiated_signal[idx]);
      } else {
        data.push_back(0.0);
      }
    }
  } else {
    data.assign(resolution * resolution, 0.0);
  }

  // Convert data to 2D PLFLT array
  std::vector<std::vector<PLFLT>> z2d(resolution,
                                      std::vector<PLFLT>(resolution));
  for (int i = 0; i < resolution; ++i)
    for (int j = 0; j < resolution; ++j)
      z2d[i][j] = static_cast<PLFLT>(data[i * resolution + j]);

  // Create pointer-to-pointer for plimage
  std::vector<PLFLT *> zptrs(resolution);
  for (int i = 0; i < resolution; ++i)
    zptrs[i] = z2d[i].data();

  // Set up plplot device and output
  plsetopt("bgcolor", "white");
  plsdev("pngcairo");
  plsfnam(output_filepath.c_str());
  plinit();

  plenv(-1.0, 1.0, -1.0, 1.0, 0, 0);
  pllab(x_gate.c_str(), y_gate.c_str(), "Stability Diagram");

  // Plot image: arguments are pointer-to-pointer, nx, ny, xmin, xmax, ymin,
  // ymax, zmin, zmax, Dxmin, Dxmax, Dymin, Dymax
  plimage(zptrs.data(), resolution, resolution, -1.0, 1.0, -1.0, 1.0, 0.0,
          1.0, // zmin, zmax (adjust as needed)
          -1.0, 1.0, -1.0, 1.0);

  plend();

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
void BuildVirtualizationMatrix(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  // Unpack configPath from params
  auto pm = unpack_params(params, param_count);
  std::string config_path = std::get<std::string>(pm.at("configPath"));
  // Load YAML and extract Cgd
  YAML::Node config = YAML::LoadFile(config_path);
  const YAML::Node &cgd = config["capacitances"]["Cgd"];
  std::vector<double> flat;
  size_t rows = cgd.size();
  size_t cols = rows > 0 ? cgd[0].size() : 0;
  flat.reserve(rows * cols);
  for (const auto &row : cgd) {
    for (const auto &v : row)
      flat.push_back(v.as<double>());
  }
  auto shape = std::array<std::size_t, 2>{rows, cols};
  xt::xarray<double> arr = xt::adapt(flat, shape);
  // Pack into FArray<double>
  auto farr = std::make_shared<falcon_core::generic::FArray<double>>(arr);
  // Pack as FFI result
  out[0] = {};
  out[0].tag = FALCON_TYPE_OPAQUE;
  out[0].value.opaque.type_name = "FArray";
  out[0].value.opaque.ptr =
      new std::shared_ptr<void>(std::static_pointer_cast<void>(farr));
  out[0].value.opaque.deleter = [](void *p) {
    delete static_cast<std::shared_ptr<void> *>(p);
  };
  *oc = 1;
}
} // extern "C"
