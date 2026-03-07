#include "qarrayDevice/Device.hpp"
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <memory>
#include <pybind11/embed.h>

// ─────────────────────────────────────────────────────────────────────────────
// Python interpreter lifetime management
//
// pybind11's scoped_interpreter calls Py_Initialize() on construction and
// Py_Finalize() on destruction.  It MUST outlive every py::object instance
// (including the Device objects created in tests, which hold a py::object
// internally).
//
// The wrong approach: a static-local singleton — C++ gives no guarantee that
// the interpreter singleton outlives Device objects created as test-local
// variables.  The debug libpython asserts on negative refcounts at shutdown.
//
// The right approach: a GoogleTest Environment whose TearDown() runs AFTER
// all test bodies have returned and all their stack-local Device objects have
// been destroyed, but BEFORE static destructors fire.
// ─────────────────────────────────────────────────────────────────────────────
class PythonEnvironment : public ::testing::Environment {
public:
  void SetUp() override {
    // Start the interpreter once for the whole test binary.
    // This must happen before any Device is constructed.
    interp_ = std::make_unique<pybind11::scoped_interpreter>();
  }

  void TearDown() override {
    // Destroy the interpreter AFTER all tests have finished.
    // At this point all test-local Device objects are already destroyed,
    // so all py::object destructors have already run safely.
    interp_.reset();
  }

private:
  // unique_ptr so we control construction/destruction timing precisely.
  std::unique_ptr<pybind11::scoped_interpreter> interp_;
};

// ─────────────────────────────────────────────────────────────────────────────
// Helpers: write a minimal YAML config to a temp file
// ─────────────────────────────────────────────────────────────────────────────
namespace {

std::string write_temp_config(const std::string &yaml_content) {
  auto tmp =
      std::filesystem::temp_directory_path() / "falcon_qarray_test_config.yaml";
  std::ofstream f(tmp);
  f << yaml_content;
  return tmp.string();
}

constexpr const char *MINIMAL_CONFIG = R"yaml(
n_dots: 2
gate_names:
  - P1
  - P2
  - B1
)yaml";

constexpr const char *SENSOR_CONFIG = R"yaml(
n_dots: 2
gate_names:
  - P1
  - P2
  - B1
  - S1
sensor_config:
  S1: {}
)yaml";

} // namespace

// ─────────────────────────────────────────────────────────────────────────────
// main: register the Python environment before running tests
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  // AddGlobalTestEnvironment takes ownership; GTest calls SetUp before any
  // test and TearDown after all tests, in the right order relative to
  // test-local variable destruction.
  ::testing::AddGlobalTestEnvironment(new PythonEnvironment());
  return RUN_ALL_TESTS();
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests
// ─────────────────────────────────────────────────────────────────────────────

TEST(DeviceConstruction, LoadsFromYamlFile) {
  auto config_path = write_temp_config(MINIMAL_CONFIG);
  ASSERT_NO_THROW({
    falcon::qarray::Device dev(config_path);
    EXPECT_EQ(dev.n_dots(), 2);
    auto gates = dev.gate_names();
    ASSERT_EQ(gates.size(), 3u);
    EXPECT_EQ(gates[0], "P1");
    EXPECT_EQ(gates[1], "P2");
    EXPECT_EQ(gates[2], "B1");
  });
}

TEST(DeviceVoltage, SetAndGetSingleGate) {
  auto config_path = write_temp_config(MINIMAL_CONFIG);
  falcon::qarray::Device dev(config_path);

  dev.set_voltage("P1", 0.5);
  auto volts = dev.voltages();
  EXPECT_DOUBLE_EQ(volts.at("P1"), 0.5);
  EXPECT_DOUBLE_EQ(volts.at("P2"), 0.0);
}

TEST(DeviceVoltage, SetMultipleGates) {
  auto config_path = write_temp_config(MINIMAL_CONFIG);
  falcon::qarray::Device dev(config_path);

  dev.set_voltages({{"P1", 0.1}, {"P2", -0.3}, {"B1", 0.8}});
  auto volts = dev.voltages();
  EXPECT_NEAR(volts.at("P1"), 0.1, 1e-9);
  EXPECT_NEAR(volts.at("P2"), -0.3, 1e-9);
  EXPECT_NEAR(volts.at("B1"), 0.8, 1e-9);
}

TEST(DeviceVoltage, InvalidGateThrows) {
  auto config_path = write_temp_config(MINIMAL_CONFIG);
  falcon::qarray::Device dev(config_path);
  EXPECT_THROW(dev.set_voltage("NOT_A_GATE", 1.0), std::exception);
}

TEST(DeviceScan1D, ReturnsChargeStates) {
  auto config_path = write_temp_config(MINIMAL_CONFIG);
  falcon::qarray::Device dev(config_path);

  auto result = dev.scan_1d("P1", {-1.0, 1.0}, 10);
  EXPECT_EQ(result.gate, "P1");
  EXPECT_EQ(result.resolution, 10);
  EXPECT_FALSE(result.has_sensor);
  EXPECT_EQ(result.charge_states.size(), 10u * 2u);
}

TEST(DeviceScan2D, ReturnsChargeStates) {
  auto config_path = write_temp_config(MINIMAL_CONFIG);
  falcon::qarray::Device dev(config_path);

  auto result = dev.scan_2d("P1", "P2", {-0.5, 0.5}, {-0.5, 0.5}, 5);
  EXPECT_EQ(result.x_gate, "P1");
  EXPECT_EQ(result.y_gate, "P2");
  EXPECT_EQ(result.resolution, 5);
  EXPECT_FALSE(result.has_sensor);
  EXPECT_EQ(result.charge_states.size(), 5u * 5u * 2u);
}

TEST(DeviceScan2D, WithSensor) {
  auto config_path = write_temp_config(SENSOR_CONFIG);
  falcon::qarray::Device dev(config_path);

  auto result = dev.scan_2d("P1", "P2", {-0.5, 0.5}, {-0.5, 0.5}, 5);
  EXPECT_TRUE(result.has_sensor);
  EXPECT_FALSE(result.sensor_output.empty());
  EXPECT_FALSE(result.differentiated_signal.empty());
}

TEST(DeviceScanRay, ReturnsTrajectory) {
  auto config_path = write_temp_config(MINIMAL_CONFIG);
  falcon::qarray::Device dev(config_path);

  auto result = dev.scan_ray({{"P1", -0.5}, {"P2", -0.5}},
                             {{"P1", 0.5}, {"P2", 0.5}}, 20);
  EXPECT_EQ(result.resolution, 20);
  EXPECT_EQ(result.trajectory.size(), 20u * 3u);
}
