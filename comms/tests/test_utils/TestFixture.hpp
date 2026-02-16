#pragma once
#include <cstdlib>
#include <gtest/gtest.h>
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

} // namespace falcon::comms::test
