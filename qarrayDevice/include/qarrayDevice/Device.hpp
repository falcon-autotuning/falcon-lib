#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

// Forward-declare the pybind11 object so consumers don't need pybind11 headers
namespace pybind11 {
class object;
}

namespace falcon::qarray {

// ─────────────────────────────────────────────────────────────────────────────
// ScanResult2D
// Returned by Device::scan_2d(). All numpy arrays are flattened row-major.
// Shapes are [resolution * resolution] for sensor_output/differentiated_signal
// and [resolution * resolution * n_dots] for charge_states.
// ─────────────────────────────────────────────────────────────────────────────
struct ScanResult2D {
  std::string x_gate;
  std::string y_gate;
  std::pair<double, double> x_range;
  std::pair<double, double> y_range;
  int resolution;

  // Always present
  std::vector<double> charge_states; // flat [res*res*n_dots]
  std::vector<int> charge_states_shape;

  // Present only when sensor is configured
  std::vector<double> sensor_output; // flat [res*res*n_sensors]
  std::vector<int> sensor_output_shape;
  std::vector<double> differentiated_signal; // flat [res*res*n_sensors]
  std::vector<int> differentiated_signal_shape;
  bool has_sensor = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// ScanResult1D
// ─────────────────────────────────────────────────────────────────────────────
struct ScanResult1D {
  std::string gate;
  std::pair<double, double> v_range;
  int resolution;

  std::vector<double> charge_states;
  std::vector<int> charge_states_shape;

  std::vector<double> sensor_output;
  std::vector<int> sensor_output_shape;
  std::vector<double> differentiated_signal;
  std::vector<int> differentiated_signal_shape;
  bool has_sensor = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// ScanResultRay
// ─────────────────────────────────────────────────────────────────────────────
struct ScanResultRay {
  std::map<std::string, double> start;
  std::map<std::string, double> end;
  int resolution;

  std::vector<double> charge_states;
  std::vector<int> charge_states_shape;

  std::vector<double> trajectory; // flat [resolution * n_gates]
  std::vector<int> trajectory_shape;

  std::vector<double> sensor_output;
  std::vector<int> sensor_output_shape;
  std::vector<double> differentiated_signal;
  std::vector<int> differentiated_signal_shape;
  bool has_sensor = false;
};

// ─────────────────────────────────────────────────────────────────────────────
// Device
//
// C++ wrapper around the Python Device class defined in device.py.
// Embeds the CPython interpreter via pybind11. Only one interpreter may be
// active per process; the first Device construction starts it.
//
// Thread safety: The GIL is acquired on every public method call.
// ─────────────────────────────────────────────────────────────────────────────
class Device {
public:
  // ── Construction ──────────────────────────────────────────────────────────

  /// Construct from a YAML config file path.
  explicit Device(const std::string &config_path);

  /// Construct from a pre-parsed config map (string→variant).
  /// Keys must match the expected YAML keys of device.py.
  /// (Convenience factory; you can pass n_dots, gate_names, etc. directly.)
  static Device from_config_path(const std::string &yaml_path);

  ~Device();

  // Movable but not copyable (pybind11 objects are reference-counted)
  Device(Device &&) noexcept;
  Device &operator=(Device &&) noexcept;
  Device(const Device &) = delete;
  Device &operator=(const Device &) = delete;

  // ── Voltage Control ───────────────────────────────────────────────────────

  void set_voltage(const std::string &gate_name, double value);
  void set_voltages(const std::map<std::string, double> &voltage_dict);

  // ── Queries ───────────────────────────────────────────────────────────────

  [[nodiscard]] int n_dots() const;
  [[nodiscard]] std::vector<std::string> gate_names() const;
  [[nodiscard]] std::map<std::string, double> voltages() const;

  // ── Scans ─────────────────────────────────────────────────────────────────

  ScanResult2D scan_2d(const std::string &x_gate, const std::string &y_gate,
                       std::pair<double, double> x_range,
                       std::pair<double, double> y_range, int resolution);

  ScanResult1D scan_1d(const std::string &gate,
                       std::pair<double, double> v_range, int resolution);

  ScanResultRay scan_ray(const std::map<std::string, double> &start,
                         const std::map<std::string, double> &end,
                         int resolution);

private:
  struct Impl;
  std::unique_ptr<Impl> impl_;
};

} // namespace falcon::qarray
