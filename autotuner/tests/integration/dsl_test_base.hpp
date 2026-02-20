// autotuner/tests/dsl/dsl_test_base.hpp
#pragma once

#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include <falcon_core/communications/Time.hpp>
#include <falcon_core/physics/config/Loader.hpp>
#include <falcon_core/physics/config/core/Config.hpp>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <thread>

namespace falcon::autotuner::test {

/**
 * @brief Base test fixture with environment setup
 */
class RoutineTestFixture : public ::testing::Test {
protected:
  void SetUp() override {
    setupEnvironment();
    std::cout << "environment variables set" << '\n';
  }

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
/**
 * @brief Test fixture for compiling .fal programs and running the interpretter
 */
class DSLTestBase : public DatabaseTestFixture {
protected:
  void SetUp() override {
    DatabaseTestFixture::SetUp();
    // Create temp directory for test files
    test_dir_ = std::filesystem::temp_directory_path() / "falcon_dsl_tests";
    std::filesystem::create_directories(test_dir_);
  }

  void TearDown() override {
    // Clean up temp files
    if (std::filesystem::exists(test_dir_)) {
      std::filesystem::remove_all(test_dir_);
    }
  }

  // Helper: Write DSL code to temp file and return path
  std::filesystem::path write_dsl_file(const std::string &content) {
    static int file_counter = 0;
    auto file_path =
        test_dir_ / ("test_" + std::to_string(file_counter++) + ".fal");

    std::ofstream out(file_path);
    EXPECT_TRUE(out.is_open()) << "Failed to create test file: " << file_path;
    out << content;
    out.close();

    return file_path;
  }

  // Helper: Compile DSL file
  static std::unique_ptr<atc::Program>
  compile_dsl(const std::filesystem::path &file_path) {
    atc::Compiler compiler;
    auto program = compiler.parse_file(file_path.string());
    EXPECT_NE(program, nullptr) << "Failed to parse: " << file_path;
    return program;
  }

  bool compile_and_run(const std::filesystem::path &dsl_file,
                       const std::string &autotuner_name, ParameterMap &params,
                       bool expect_success = true) {
    return compile_and_run(std::vector<std::filesystem::path>{dsl_file},
                           autotuner_name, params, expect_success);
  }
  bool compile_and_run(const std::vector<std::filesystem::path> &dsl_files,
                       const std::string &autotuner_name, ParameterMap &params,
                       bool expect_success = true) {
    // Concatenate all DSL files into one string
    std::stringstream dsl_code;
    for (const auto &file : dsl_files) {
      std::ifstream in(file);
      if (!in.is_open()) {
        std::cerr << "Failed to open DSL file: " << file << '\n';
        return false;
      }
      dsl_code << in.rdbuf() << "\n";
    }
    // Use the existing compile_and_run logic with the concatenated string
    return compile_and_run(dsl_code.str(), autotuner_name, params,
                           expect_success);
  }
  // Helper: Compile and run in one step
  bool compile_and_run(const std::string &dsl_code,
                       const std::string &autotuner_name, ParameterMap &params,
                       bool expect_success = true) {
    std::atomic<bool> autotuner_result{false};
    std::atomic<bool> responder_ready{false};
    std::atomic<bool> request_received{false};

    auto file_path = write_dsl_file(dsl_code);
    auto program = compile_dsl(file_path);
    if (!program) {
      return false;
    }
    // Start responder thread
    std::thread responder([&]() {
      auto &hub = comms::NatsManager::instance();
      hub.subscribe(
          "INSTRUMENTHUB.DEVICE_CONFIG_REQUEST",
          [&](const std::string & /*msg*/) {
            request_received = true;
            DeviceConfigResponse response;
            response.timestamp =
                (int)falcon_core::communications::Time().time();
            falcon_core::physics::config::Loader loader(
                std::filesystem::path(__FILE__).parent_path().parent_path() /
                "device-configs/three-dot.yml");
            const falcon_core::physics::config::core::ConfigSP config =
                loader.config();
            response.response = config->to_json_string();

            nlohmann::json j = response.to_json();
            hub.publish("FALCON.DEVICE_CONFIG_RESPONSE", j.dump());
          });
      responder_ready = true;
      std::this_thread::sleep_for(std::chrono::seconds(1));
      hub.unsubscribe("INSTRUMENTHUB.DEVICE_CONFIG_REQUEST");
    });

    while (!responder_ready) {
      std::this_thread::yield();
    }
    std::thread client([&]() {
      try {
        autotuner_result =
            run_autotuner(*program, autotuner_name, params, expect_success);
      } catch (const std::exception &e) {
        std::cout << "EXCEPTION in client thread: " << e.what() << '\n';
        throw;
      }
    });

    client.join();
    responder.join();

    return autotuner_result;
  }

  std::filesystem::path test_dir_;

private:
  // Helper: Run autotuner with given parameters
  static bool run_autotuner(const atc::Program &program,
                            const std::string &autotuner_name,
                            ParameterMap &params, bool expect_success = true) {
    Interpreter interpreter(program);
    bool result = interpreter.run(autotuner_name, params);

    if (expect_success) {
      EXPECT_TRUE(result) << "Autotuner execution failed: " << autotuner_name;
    } else {
      EXPECT_FALSE(result) << "Autotuner should have failed: "
                           << autotuner_name;
    }

    return result;
  }
};

} // namespace falcon::autotuner::test
