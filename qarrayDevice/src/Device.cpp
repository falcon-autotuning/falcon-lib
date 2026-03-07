#include "qarrayDevice/Device.hpp"

#include <filesystem>
#include <mutex>
#include <stdexcept>
#include <string>

// pybind11 embedding — includes the Python.h implicitly
#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

namespace py = pybind11;
using namespace pybind11::literals;

namespace falcon::qarray {

// ─────────────────────────────────────────────────────────────────────────────
// Interpreter singleton
//
// pybind11::scoped_interpreter must be constructed exactly once per process.
// We use a Meyer's singleton with a once_flag so that creating multiple
// Device objects in the same process is safe.
// ─────────────────────────────────────────────────────────────────────────────
namespace {

struct InterpreterGuard {
  static InterpreterGuard &instance() {
    static InterpreterGuard guard;
    return guard;
  }

private:
  py::scoped_interpreter interp_;
  InterpreterGuard() : interp_() {}
  ~InterpreterGuard() = default;
};

// Convert a numpy array to a flat std::vector<double> and return the shape.
std::pair<std::vector<double>, std::vector<int>>
numpy_to_vector(const py::object &arr) {
  auto np_arr = arr.cast<
      py::array_t<double, py::array::c_style | py::array::forcecast>>();
  auto buf = np_arr.request();

  std::vector<int> shape;
  shape.reserve(buf.ndim);
  for (py::ssize_t i = 0; i < buf.ndim; ++i) {
    shape.push_back(static_cast<int>(buf.shape[i]));
  }

  const auto *ptr = static_cast<const double *>(buf.ptr);
  std::vector<double> data(ptr, ptr + buf.size);
  return {data, shape};
}

// Convert a numpy int array to flat std::vector<double> via cast-to-double.
std::pair<std::vector<double>, std::vector<int>>
numpy_int_to_double_vector(const py::object &arr) {
  // charge_states may be int-typed; cast to float64 first
  py::object float_arr =
      py::module_::import("numpy").attr("array")(arr, "dtype"_a = "float64");
  return numpy_to_vector(float_arr);
}

} // anonymous namespace

// ─────────────────────────────────────────────────────────────────────────────
// Impl
// ─────────────────────────────────────────────────────────────────────────────
struct Device::Impl {
  py::object py_device; // The Python Device instance

  explicit Impl(const std::string &config_path) {
    // Ensure the interpreter is running
    InterpreterGuard::instance();

    py::gil_scoped_acquire gil;

    // Locate the quantum_dot_device.py bundled alongside this library.
    // It lives next to this compiled .so in the install tree, or at the
    // path set by FALCON_QARRAY_PYTHON_PATH env var.
    std::string module_dir;
    if (const char *env = std::getenv("FALCON_QARRAY_PYTHON_PATH")) {
      module_dir = env;
    } else {
      // Default: same directory as the shared library.
      // __file__ equivalent: look relative to source location at build time.
      // We embed the path at cmake configure time via a generated header.
#ifdef FALCON_QARRAY_PYTHON_MODULE_DIR
      module_dir = FALCON_QARRAY_PYTHON_MODULE_DIR;
#else
      module_dir = "/opt/falcon/share/qarrayDevice";
#endif
    }

    // Insert module directory into sys.path
    py::module_::import("sys").attr("path").attr("insert")(0, module_dir);

    // Import the module and instantiate Device
    py::module_ qd_module = py::module_::import("quantum_dot_device");
    py::object DeviceClass = qd_module.attr("Device");
    py_device = DeviceClass(config_path);
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// Device public API
// ─────────────────────────────────────────────────────────────────────────────

Device::Device(const std::string &config_path)
    : impl_(std::make_unique<Impl>(config_path)) {}

Device Device::from_config_path(const std::string &yaml_path) {
  return Device(yaml_path);
}

Device::~Device() = default;
Device::Device(Device &&) noexcept = default;
Device &Device::operator=(Device &&) noexcept = default;

void Device::set_voltage(const std::string &gate_name, double value) {
  py::gil_scoped_acquire gil;
  impl_->py_device.attr("set_voltage")(gate_name, value);
}

void Device::set_voltages(const std::map<std::string, double> &voltage_dict) {
  py::gil_scoped_acquire gil;
  impl_->py_device.attr("set_voltages")(voltage_dict);
}

int Device::n_dots() const {
  py::gil_scoped_acquire gil;
  return impl_->py_device.attr("n_dots").cast<int>();
}

std::vector<std::string> Device::gate_names() const {
  py::gil_scoped_acquire gil;
  return impl_->py_device.attr("gate_names").cast<std::vector<std::string>>();
}

std::map<std::string, double> Device::voltages() const {
  py::gil_scoped_acquire gil;
  return impl_->py_device.attr("voltages")
      .cast<std::map<std::string, double>>();
}

// ── scan_2d ──────────────────────────────────────────────────────────────────

ScanResult2D Device::scan_2d(const std::string &x_gate,
                             const std::string &y_gate,
                             std::pair<double, double> x_range,
                             std::pair<double, double> y_range,
                             int resolution) {
  py::gil_scoped_acquire gil;

  py::tuple py_x_range = py::make_tuple(x_range.first, x_range.second);
  py::tuple py_y_range = py::make_tuple(y_range.first, y_range.second);

  py::dict result = impl_->py_device.attr("scan_2d")(x_gate, y_gate, py_x_range,
                                                     py_y_range, resolution);

  ScanResult2D out;
  out.x_gate = x_gate;
  out.y_gate = y_gate;
  out.x_range = x_range;
  out.y_range = y_range;
  out.resolution = resolution;

  auto [cs, cs_shape] = numpy_int_to_double_vector(result["charge_states"]);
  out.charge_states = std::move(cs);
  out.charge_states_shape = std::move(cs_shape);

  if (result.contains("sensor_output")) {
    out.has_sensor = true;
    auto [so, so_shape] = numpy_to_vector(result["sensor_output"]);
    out.sensor_output = std::move(so);
    out.sensor_output_shape = std::move(so_shape);

    auto [ds, ds_shape] = numpy_to_vector(result["differentiated_signal"]);
    out.differentiated_signal = std::move(ds);
    out.differentiated_signal_shape = std::move(ds_shape);
  }

  return out;
}

// ── scan_1d ──────────────────────────────────────────────────────────────────

ScanResult1D Device::scan_1d(const std::string &gate,
                             std::pair<double, double> v_range,
                             int resolution) {
  py::gil_scoped_acquire gil;

  py::tuple py_v_range = py::make_tuple(v_range.first, v_range.second);

  py::dict result =
      impl_->py_device.attr("scan_1d")(gate, py_v_range, resolution);

  ScanResult1D out;
  out.gate = gate;
  out.v_range = v_range;
  out.resolution = resolution;

  auto [cs, cs_shape] = numpy_int_to_double_vector(result["charge_states"]);
  out.charge_states = std::move(cs);
  out.charge_states_shape = std::move(cs_shape);

  if (result.contains("sensor_output")) {
    out.has_sensor = true;
    auto [so, so_shape] = numpy_to_vector(result["sensor_output"]);
    out.sensor_output = std::move(so);
    out.sensor_output_shape = std::move(so_shape);

    auto [ds, ds_shape] = numpy_to_vector(result["differentiated_signal"]);
    out.differentiated_signal = std::move(ds);
    out.differentiated_signal_shape = std::move(ds_shape);
  }

  return out;
}

// ── scan_ray ─────────────────────────────────────────────────────────────────

ScanResultRay Device::scan_ray(const std::map<std::string, double> &start,
                               const std::map<std::string, double> &end,
                               int resolution) {
  py::gil_scoped_acquire gil;

  py::dict result = impl_->py_device.attr("scan_ray")(start, end, resolution);

  ScanResultRay out;
  out.start = start;
  out.end = end;
  out.resolution = resolution;

  auto [cs, cs_shape] = numpy_int_to_double_vector(result["charge_states"]);
  out.charge_states = std::move(cs);
  out.charge_states_shape = std::move(cs_shape);

  auto [traj, traj_shape] = numpy_to_vector(result["trajectory"]);
  out.trajectory = std::move(traj);
  out.trajectory_shape = std::move(traj_shape);

  if (result.contains("sensor_output")) {
    out.has_sensor = true;
    auto [so, so_shape] = numpy_to_vector(result["sensor_output"]);
    out.sensor_output = std::move(so);
    out.sensor_output_shape = std::move(so_shape);

    auto [ds, ds_shape] = numpy_to_vector(result["differentiated_signal"]);
    out.differentiated_signal = std::move(ds);
    out.differentiated_signal_shape = std::move(ds_shape);
  }

  return out;
}

} // namespace falcon::qarray
