#pragma once
#include "falcon-database/DatabaseConnection.hpp"
#include <cstdlib>
#include <gtest/gtest.h>
#include <string>

namespace falcon::routine::test {

/**
 * @brief Base test fixture with environment setup
 */
class RoutineTestFixture : public ::testing::Test {
protected:
  void SetUp() override { setupEnvironment(); }

  void TearDown() override {
    // Clean up environment variables to avoid pollution
    unsetenv("FALCON_DATABASE_URL");
    unsetenv("NATS_URL");
  }

  void setupEnvironment() {
    // Database URL from TEST_DATABASE_URL environment variable
    const char *test_db_url = std::getenv("TEST_DATABASE_URL");
    if (test_db_url == nullptr) {
      // Default for tests - use 127.0.0.1 to force TCP connection
      db_url_ = "postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/"
                "falcon_test";
    } else {
      db_url_ = test_db_url;
    }

    // Set FALCON_DATABASE_URL for database connections that use env var
    setenv("FALCON_DATABASE_URL", db_url_.c_str(), 1);

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
    setenv("LOG_LEVEL", "debug", 1);
  }

  [[nodiscard]] std::string getDatabaseUrl() const { return db_url_; }
  [[nodiscard]] std::string getNatsUrl() const { return nats_url_; }

  std::string db_url_;
  std::string nats_url_;
};

/**
 * @brief Test fixture with database connection
 */
class DatabaseTestFixture : public RoutineTestFixture {
protected:
  void SetUp() override {
    RoutineTestFixture::SetUp();

    // Create database connection with explicit URL
    // This ensures tests don't accidentally use production database
    db_ = std::make_unique<falcon::database::AdminDatabaseConnection>(
        getDatabaseUrl());
    db_->initialize_schema();
    db_->clear_all();
  }

  void TearDown() override {
    if (db_) {
      db_->clear_all();
    }
    RoutineTestFixture::TearDown();
  }

  std::unique_ptr<falcon::database::AdminDatabaseConnection> db_;
};

} // namespace falcon::routine::test
