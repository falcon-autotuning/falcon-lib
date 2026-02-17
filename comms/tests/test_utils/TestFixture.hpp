#pragma once

#include "falcon-comms/natsManager.hpp"
#include <cstdlib>
#include <gtest/gtest.h>
#include <nats/nats.h>
#include <string>

namespace falcon::comms::test {

/**
 * @brief Base test fixture with environment setup
 */
class CommsTestFixture : public ::testing::Test {
protected:
  void SetUp() override { setupEnvironment(); }

  void TearDown() override {
    // Clean up environment variables to avoid pollution
    unsetenv("NATS_URL");

    // Force disconnect to clean up resources properly
    // This ensures cleanup happens while NATS library is still valid
    try {
      NatsManager::instance().disconnect();
    } catch (...) {
      // Ignore errors during test cleanup
    }
  }

  void setupEnvironment() {
    // NATS URL from environment or default
    const char *test_nats_url = std::getenv("TEST_NATS_URL");
    if (test_nats_url == nullptr) {
      nats_url_ = "nats://localhost:4222";
    } else {
      nats_url_ = test_nats_url;
    }

    // Set NATS_URL for Hub connections
    setenv("NATS_URL", nats_url_.c_str(), 1);

    // Set log level to debug for tests
    setenv("SPDLOG_LEVEL", "debug", 1);
  }

  [[nodiscard]] std::string getNatsUrl() const { return nats_url_; }

private:
  std::string nats_url_;
};

/**
 * @brief Global test environment to handle NATS library shutdown
 */
class NatsTestEnvironment : public ::testing::Environment {
public:
  ~NatsTestEnvironment() override = default;

  void SetUp() override {
    // Library is initialized on first use by NatsManager
  }

  void TearDown() override {
    // Clean up NATS library after all tests complete
    nats_Close();
  }
};

} // namespace falcon::comms::test
