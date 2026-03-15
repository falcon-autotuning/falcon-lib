#include "falcon-dsl/ParameterMap.hpp"
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

// Mock hardware device
class MockDevice {
public:
  bool initialize() {
    std::cout << "[HW] Device initialized\n";
    return true;
  }

  void set_voltage(float v) { voltage_ = v; }

  float measure_current() {
    // Simulate I-V curve: I = sin(V * pi) * 1e-6
    return std::sin(voltage_ * 3.14159f) * 1e-6f;
  }

  bool reset() {
    std::cout << "[HW] Device reset\n";
    return true;
  }

private:
  float voltage_ = 0.0f;
};

static MockDevice g_device;
static std::vector<std::pair<float, float>> g_measurements;

namespace VoltageSweep {

using namespace falcon::dsl;

ParameterMap initialize_device(const ParameterMap &params) {
  ParameterMap result;

  std::cout << "=== Initializing Device ===\n";

  bool success = g_device.initialize();
  g_measurements.clear();

  result.set("init_success", success);
  result.set("error_msg",
             success ? std::string("") : std::string("Initialization failed"));

  return result;
}

ParameterMap prepare_sweep(const ParameterMap &params) {
  ParameterMap result;

  float min_v = params.get<float>("min_voltage");
  float max_v = params.get<float>("max_voltage");
  float step = params.get<float>("step");

  std::cout << "\n=== Preparing Sweep ===\n";
  std::cout << "Range: " << min_v << " V to " << max_v << " V\n";
  std::cout << "Step: " << step << " V\n\n";

  return result;
}

ParameterMap measure_iv(const ParameterMap &params) {
  ParameterMap result;

  float voltage = params.get<float>("current_voltage");

  g_device.set_voltage(voltage);

  // Simulate measurement delay
  // std::this_thread::sleep_for(std::chrono::milliseconds(10));

  float current = g_device.measure_current();

  std::cout << "V = " << voltage << " V  →  I = " << current << " A\n";

  g_measurements.push_back({voltage, current});

  result.set("measured_current", current);
  result.set("measured_voltage", voltage);
  result.set("valid", true);

  return result;
}

ParameterMap handle_saturation(const ParameterMap &params) {
  ParameterMap result;

  float threshold = params.get<float>("current_threshold");

  std::cout << "\n! Saturation detected (I > " << threshold << " A)\n";
  std::cout << "  Increasing threshold...\n\n";

  result.set("recovery_success", true);

  return result;
}

ParameterMap save_data(const ParameterMap &params) {
  ParameterMap result;

  int count = params.get<int>("measurement_count");

  std::cout << "\n=== Saving Data ===\n";
  std::cout << "Total measurements: " << count << "\n";

  std::ofstream file("iv_curve.csv");
  if (!file.is_open()) {
    result.set("save_success", false);
    return result;
  }

  file << "Voltage (V),Current (A)\n";
  for (const auto &[v, i] : g_measurements) {
    file << v << "," << i << "\n";
  }
  file.close();

  std::cout << "Saved to: iv_curve.csv\n";

  result.set("save_success", true);

  return result;
}

} // namespace VoltageSweep
