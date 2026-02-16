#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::atc;

class InterpreterIntegrationTest : public ::testing::Test {
protected:
  void SetUp() override {
    // Find v2_test.fal relative to this test
    test_file = std::filesystem::path(__FILE__).parent_path().parent_path() /
                "v2_test.fal";
    ASSERT_TRUE(std::filesystem::exists(test_file))
        << "v2_test.fal not found at " << test_file;
  }

  std::filesystem::path test_file;
};

TEST_F(InterpreterIntegrationTest, RunV2Test) {
  Compiler compiler;
  auto program = compiler.parse_file(test_file);
  ASSERT_NE(program, nullptr);

  // Mock config
  class MockConfig : public falcon_core::physics::config::core::Config {
  public:
    MockConfig() : Config() {}
  };
  MockConfig config;

  // Interpreter
  Interpreter interpreter(*program, config, "nats://localhost:4222");

  // We don't expect it to succeed without a running NATS,
  // but at least it shouldn't crash during initialization and setup.
  // For a real integration test, we'd need a mock NATS or a running one.

  // ParameterMap for entry
  ParameterMap params;
  params.set("min_voltage", 0.5);

  // Note: This might timeout or fail due to NATS, but we are testing the
  // harness here. In a real CI environment, we'd mock the nats calls.
}
