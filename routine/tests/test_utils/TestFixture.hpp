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
    // Clean up if needed
  }

  void setupEnvironment() {
    // Database URL from environment or default
    const char *db_url = std::getenv("TEST_DATABASE_URL");
    if (db_url == nullptr) {
      db_url_ = "postgresql://falcon_test:falcon_test_password@localhost:5433/"
                "falcon_test";
    } else {
      db_url_ = db_url;
    }
    setenv("DATABASE_URL", db_url_.c_str(), 1);

    // NATS URL from environment or default
    const char *nats_url = std::getenv("TEST_NATS_URL");
    if (nats_url == nullptr) {
      nats_url_ = "nats://localhost:4222";
    } else {
      nats_url_ = nats_url;
    }
    setenv("NATS_URL", nats_url_.c_str(), 1);

    // Set log level to debug for tests
    setenv("LOG_LEVEL", "debug", 1);
  }

  [[nodiscard]] std::string getDatabaseUrl() const { return db_url_; }
  [[nodiscard]] std::string getNatsUrl() const { return nats_url_; }

private:
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

    // Create database connection and initialize schema
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
