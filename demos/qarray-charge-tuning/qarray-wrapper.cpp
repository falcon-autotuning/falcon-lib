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
#include <falcon_core/physics/device_structures/Connections.hpp>
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
static std::unique_ptr<pybind11::scoped_interpreter> g_interpreter;

static void ensure_python_interpreter() {
  if (!python_initialized) {
    dlopen("/opt/falcon/lib/libpython3.12.so", RTLD_NOW | RTLD_GLOBAL);
    g_interpreter = std::make_unique<pybind11::scoped_interpreter>();
    python_initialized = true;
    spdlog::info("Python interpreter initialized for qarray-wrapper");
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
using Connections = falcon_core::physics::device_structures::Connections;
using ConnectionsSP = std::shared_ptr<Connections>;
using FArray = falcon_core::generic::FArray<double>;
using FArraySP = std::shared_ptr<FArray>;
using Quantity = falcon_core::math::Quantity;
using QuantitySP = std::shared_ptr<Quantity>;
using SymbolUnit = falcon_core::physics::units::SymbolUnit;
using SymbolUnitSP = std::shared_ptr<SymbolUnit>;
using Point = falcon_core::math::Point;
using PointSP = std::shared_ptr<Point>;
using QDevice = falcon::qarray::Device;

// ── Module-level singleton device
// ─────────────────────────────────────────────────────

static QDevice *g_device = nullptr;
static std::string current_config_path;

static void initialize_device_internal(const std::string &config_path) {
  spdlog::info("Initializing device with config: {}", config_path);
  ensure_python_interpreter();

  if (g_device != nullptr) {
    spdlog::debug("Deleting existing device instance");
    delete g_device;
    g_device = nullptr;
  }

  try {
    g_device = new QDevice(config_path);
    current_config_path = config_path;
    spdlog::info("Device successfully initialized with {} gates",
                 g_device->gate_names().size());
  } catch (const std::exception &e) {
    spdlog::error("Failed to initialize device with config {}: {}", config_path,
                  e.what());
    if (g_device != nullptr) {
      delete g_device;
      g_device = nullptr;
    }
    throw;
  }
}

static void initialize_device() {
  ensure_python_interpreter();
  const char *env_path = std::getenv("QARRAY_CONFIG_PATH");
#ifndef QARRAY_CONFIG
#define QARRAY_CONFIG "/opt/qarray/device.yaml"
#endif
  std::string config_path = env_path ? env_path : QARRAY_CONFIG;

  // Only initialize if not already initialized with this path
  if (g_device != nullptr && current_config_path == config_path) {
    spdlog::debug("Device already initialized with {}", config_path);
    return;
  }

  initialize_device_internal(config_path);
}

static QDevice &device() {
  if (g_device == nullptr) {
    initialize_device();
  }
  return *g_device;
}

extern "C" void RestartDevice(const FalconParamEntry *params,
                              int32_t param_count, FalconResultSlot *out,
                              int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string config_path = std::get<std::string>(pm.at("configPath"));

  try {
    initialize_device_internal(config_path);
    out[0] = {};
    out[0].tag = FALCON_TYPE_NIL;
    *oc = 1;
  } catch (const std::exception &e) {
    spdlog::error("RestartDevice failed: {}", e.what());
    out[0] = {};
    out[0].tag = FALCON_TYPE_NIL;
    *oc = 1;
    throw;
  }
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

static std::shared_ptr<ArrayValue>
get_struct_field_array(const std::shared_ptr<StructInstance> &inst,
                       const char *field_name) {
  if (!inst) {
    throw std::runtime_error(
        std::string("get_struct_field_array: null StructInstance for field '") +
        field_name + "'");
  }

  auto it = inst->fields->find(field_name);
  if (it == inst->fields->end()) {
    throw std::runtime_error(std::string("get_struct_field_array: field '") +
                             field_name + "' not found in StructInstance");
  }
  const RuntimeValue &fv = it->second;
  if (std::holds_alternative<std::shared_ptr<StructInstance>>(fv)) {
    auto farr_inst = std::get<std::shared_ptr<StructInstance>>(fv);
    if (farr_inst && farr_inst->native_handle.has_value()) {
      return std::static_pointer_cast<ArrayValue>(
          farr_inst->native_handle.value());
    }
  }
  throw std::runtime_error(std::string("get_struct_field_array: field '") +
                           field_name +
                           "' is not a StructInstance with a native_handle");
}

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

static std::map<std::string, double>
dsl_map_to_volt_map(const std::shared_ptr<StructInstance> &map_inst) {
  auto keys_arr = get_struct_field_array(map_inst, "keys_");
  auto vals_arr = get_struct_field_array(map_inst, "values_");

  if (keys_arr->elements.size() != vals_arr->elements.size()) {
    throw std::runtime_error(
        "qarray-wrapper: Map<Connection,Quantity> keys_ and values_ "
        "have different sizes");
  }

  auto volt_unit = SymbolUnit::Volt();
  std::map<std::string, double> result;

  for (size_t i = 0; i < keys_arr->elements.size(); ++i) {
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

    auto qty_copy = std::make_shared<Quantity>(*qty);
    qty_copy->convert_to(volt_unit);
    double volts = qty_copy->value();

    result[conn->name()] = volts;
  }
  return result;
}

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

static std::vector<int> detect_peaks(const std::vector<double> &signal,
                                     double threshold_sigma = 2.0) {
  const int n = static_cast<int>(signal.size());
  if (n < 3) {
    return {};
  }

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

// Helper: Get a StructInstance from the unpacked ParameterMap
static std::shared_ptr<StructInstance>
get_struct_instance(const ParameterMap &pm, const std::string &key) {
  auto it = pm.find(key);
  if (it == pm.end()) {
    throw std::runtime_error("Parameter '" + key + "' not found");
  }
  if (!std::holds_alternative<std::shared_ptr<StructInstance>>(it->second)) {
    throw std::runtime_error("Parameter '" + key + "' is not a StructInstance");
  }
  return std::get<std::shared_ptr<StructInstance>>(it->second);
}

// ─────────────────────────────────────────────────────────────────────────────
// Exported FFI functions
// ────────────────────────────────────────────────────────────────────────────

extern "C" {

void CollectCurrentDeviceState(const FalconParamEntry * /*params*/,
                               int32_t /*param_count*/, FalconResultSlot *out,
                               int32_t *oc) {
  const auto &volt_map = device().voltages();
  auto dvss = build_dvss_from_volt_map(volt_map);
  pack_dvss(std::move(dvss), out, oc);
}

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

void Ramp(const FalconParamEntry *params, int32_t param_count,
          FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  auto stop_inst = get_struct_instance(pm, "stop");
  auto stop_map = dsl_map_to_volt_map(stop_inst);

  auto start_map = device().voltages();

  constexpr int ramp_resolution = 4;
  device().scan_ray(start_map, stop_map, ramp_resolution);

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

void AnalyzeBlips(const FalconParamEntry *params, int32_t param_count,
                  FalconResultSlot *out, int32_t *oc) {
  auto pm = unpack_params(params, param_count);

  auto start_inst = get_struct_instance(pm, "begin");
  auto stop_inst = get_struct_instance(pm, "stop");
  int resolution = static_cast<int>(std::get<int64_t>(pm.at("resolution")));

  auto start_map = dsl_map_to_volt_map(start_inst);
  auto stop_map = dsl_map_to_volt_map(stop_inst);

  auto result = device().scan_ray(start_map, stop_map, resolution);

  std::vector<double> signal;
  signal.reserve(static_cast<size_t>(resolution));

  if (result.has_sensor && !result.differentiated_signal.empty()) {
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
      auto idx = static_cast<size_t>(i * n_sensors);
      if (idx < result.differentiated_signal.size()) {
        signal.push_back(result.differentiated_signal[idx]);
      } else {
        signal.push_back(0.0);
      }
    }
  } else {
    signal.assign(static_cast<size_t>(resolution), 0.0);
  }

  auto peak_indices = detect_peaks(signal);

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

  int num_blips = static_cast<int>(peak_indices.size());

  out[0] = {};
  out[0].tag = FALCON_TYPE_INT;
  out[0].value.int_val = static_cast<int64_t>(num_blips);

  {
    FalconResultSlot tmp[1];
    int32_t tmp_oc = 0;
    pack_dvss_array(std::move(locations), tmp, &tmp_oc);
    out[1] = tmp[0];
  }

  *oc = 2;
}

void EndState(const FalconParamEntry *params, int32_t param_count,
              FalconResultSlot *out, int32_t *oc) {
  try {
    auto direction =
        get_opaque_param<Connection>(params, param_count, "direction");
    auto spacing = get_opaque_param<Quantity>(params, param_count, "spacing");
    auto virtualization_arr =
        get_opaque_param<FArray>(params, param_count, "virtualization");
    auto virtual_names_arr =
        get_opaque_param<Connections>(params, param_count, "virtualNames");

    std::vector<ConnectionSP> virtual_names = virtual_names_arr->items();
    const int n_gates = static_cast<int>(virtual_names.size());

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
      std::string available_names;
      for (const auto &conn : virtual_names) {
        available_names += fmt::format("[{}:{}] ", conn->name(), conn->type());
      }
      throw std::runtime_error(fmt::format(
          "EndState: direction Connection not found in virtualNames. The "
          "current direction is {} and the type is {}. The virtualNames that "
          "are available are {}",
          direction->name(), direction->type(), available_names));
    }

    xt::xarray<double> virt_matrix = virtualization_arr->data();
    xt::xarray<double> identity = xt::eye<double>(n_gates);
    for (int i = 0; i < n_gates; ++i) {
      int pivot_row = i;
      for (int j = i + 1; j < n_gates; ++j) {
        if (std::abs(virt_matrix(j, i)) > std::abs(virt_matrix(pivot_row, i))) {
          pivot_row = j;
        }
      }
      if (pivot_row != i) {
        xt::view(virt_matrix, i) = xt::view(virt_matrix, pivot_row);
        xt::view(virt_matrix, pivot_row) = xt::view(virt_matrix, i);
        xt::view(identity, i) = xt::view(identity, pivot_row);
        xt::view(identity, pivot_row) = xt::view(identity, i);
      }
      if (std::abs(virt_matrix(i, i)) < 1e-10) {
        throw std::runtime_error("Matrix is singular or ill-conditioned");
      }
      double pivot = virt_matrix(i, i);
      auto row = xt::view(virt_matrix, i, xt::all());
      row = row / pivot;
      auto id_row = xt::view(identity, i, xt::all());
      id_row = id_row / pivot;
      for (int j = 0; j < n_gates; ++j) {
        if (i != j) {
          double factor = virt_matrix(j, i);
          auto row_j = xt::view(virt_matrix, j, xt::all());
          auto row_i = xt::view(virt_matrix, i, xt::all());
          row_j = row_j - factor * row_i;

          auto id_row_j = xt::view(identity, j, xt::all());
          auto id_row_i = xt::view(identity, i, xt::all());
          id_row_j = id_row_j - factor * id_row_i;
        }
      }
    }

    std::vector<double> result_vector(n_gates, 0.0);
    for (int i = 0; i < n_gates; ++i) {
      for (int j = 0; j < n_gates; ++j) {
        result_vector[i] += identity(i, j) * unit_vector[j];
      }
    }

    auto volt_unit = SymbolUnit::Volt();
    auto spacing_copy = std::make_shared<Quantity>(*spacing);
    spacing_copy->convert_to(volt_unit);
    double spacing_value = spacing_copy->value();

    for (double &v : result_vector) {
      v *= spacing_value;
    }

    const auto &current_voltages = device().voltages();
    auto adjustment = std::make_shared<Point>();

    for (int i = 0; i < n_gates; ++i) {
      auto conn = virtual_names[i];
      auto qty = std::make_shared<Quantity>(result_vector[i], volt_unit);
      adjustment->insert(conn, qty);
    }

    auto current_state = build_dvss_from_volt_map(current_voltages);
    auto current_point = current_state->to_point();

    auto final_point = current_point->operator+(adjustment);

    pack_point(std::move(final_point), out, oc);

  } catch (const std::exception &e) {
    out[0] = {};
    out[0].tag = FALCON_TYPE_NIL;
    *oc = 1;
    throw;
  }
}

void plot_stability_diagram(const std::string &filename,
                            const std::vector<double> &data, int resolution,
                            double xmin, double xmax, double ymin, double ymax,
                            const std::string &xlabel,
                            const std::string &ylabel) {
  if (data.empty())
    return;

  std::vector<std::vector<PLFLT>> z2d(resolution,
                                      std::vector<PLFLT>(resolution));
  for (int i = 0; i < resolution; ++i)
    for (int j = 0; j < resolution; ++j)
      z2d[i][j] = static_cast<PLFLT>(data[i * resolution + j]);

  std::vector<PLFLT *> zptrs(resolution);
  for (int i = 0; i < resolution; ++i)
    zptrs[i] = z2d[i].data();

  plsetopt("bgcolor", "white");
  plsdev("pngcairo");
  plsfnam(filename.c_str());
  plinit();
  plenv(xmin, xmax, ymin, ymax, 0, 0);
  pllab(xlabel.c_str(), ylabel.c_str(), "Charge Stability Diagram");

  // Find min/max for scaling
  double zmin = data[0], zmax = data[0];
  for (double v : data) {
    if (v < zmin)
      zmin = v;
    if (v > zmax)
      zmax = v;
  }
  if (zmin == zmax)
    zmax += 1.0;

  plimage(zptrs.data(), resolution, resolution, xmin, xmax, ymin, ymax, zmin,
          zmax, xmin, xmax, ymin, ymax);
  plend();
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
  double plot_range = 2.0;
  auto result = device().scan_2d(x_gate, y_gate, {-2 * plot_range, plot_range},
                                 {-plot_range, plot_range}, resolution);
  std::vector<double> data;
  if (result.has_sensor && !result.differentiated_signal.empty()) {
    data = result.differentiated_signal;
  } else {
    data.assign(resolution * resolution, 0.0);
  }

  plot_stability_diagram(output_filepath, data, resolution, -2 * plot_range,
                         plot_range, -plot_range, plot_range, x_gate + " (V)",
                         y_gate + " (V)");

  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}

void BuildVirtualizationMatrix(const FalconParamEntry *params,
                               int32_t param_count, FalconResultSlot *out,
                               int32_t *oc) {
  auto pm = unpack_params(params, param_count);
  std::string config_path = std::get<std::string>(pm.at("configPath"));
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
  auto farr = std::make_shared<falcon_core::generic::FArray<double>>(arr);
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
