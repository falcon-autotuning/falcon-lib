// autotuner/tests/dsl/dsl_test_base.hpp
#pragma once

#include "falcon-atc/Compiler.hpp"
#include "falcon-autotuner/Interpreter.hpp"
#include "falcon-autotuner/ParameterMap.hpp"
#include "falcon-database/SnapshotManager.hpp"
#include <falcon_core/communications/Time.hpp>
#include <falcon_core/physics/config/Loader.hpp>
#include <falcon_core/physics/config/core/Config.hpp>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <thread>
#include <utility>

namespace falcon::autotuner::test {
struct CompileEnvironment {
  std::vector<std::filesystem::path> dsl_files;
  std::string autotuner_name;
  ParameterMap &params;
  bool expect_success;
  std::optional<std::filesystem::path> globals;
  std::filesystem::path device_config;
  std::optional<std::vector<std::filesystem::path>> routine_libs;

  CompileEnvironment(
      std::vector<std::filesystem::path> dsl_files_,
      std::string autotuner_name_, ParameterMap &params_,
      std::optional<bool> expect_success_ = true,
      std::optional<std::filesystem::path> globals_ = std::nullopt,
      std::filesystem::path device_config_ =
          std::filesystem::path(__FILE__).parent_path().parent_path() /
          "device-configs/three-dot.yml",
      std::optional<std::vector<std::filesystem::path>> routine_libs_ =
          std::nullopt)
      : dsl_files(std::move(dsl_files_)),
        autotuner_name(std::move(autotuner_name_)), params(params_),
        expect_success(expect_success_), globals(std::move(globals_)),
        device_config(std::move(device_config_)),
        routine_libs(std::move(routine_libs_)) {}
};
struct SingleCompileEnvironment : public CompileEnvironment {
  SingleCompileEnvironment(
      std::filesystem::path dsl_file_, std::string autotuner_name_,
      ParameterMap &params_, bool expect_success_ = true,
      std::optional<std::filesystem::path> globals_ = std::nullopt,
      std::filesystem::path device_config_ =
          std::filesystem::path(__FILE__).parent_path().parent_path() /
          "device-configs/three-dot.yml",
      std::optional<std::vector<std::filesystem::path>> routine_libs_ =
          std::nullopt)
      : CompileEnvironment({std::move(dsl_file_)}, std::move(autotuner_name_),
                           params_, expect_success_, std::move(globals_),
                           std::move(device_config_),
                           std::move(routine_libs_)) {}
};

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
    db_ = std::make_shared<falcon::database::AdminDatabaseConnection>(
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

  std::shared_ptr<falcon::database::AdminDatabaseConnection> db_;
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
  // Helper: Compile and run in one step
  bool compile_and_run(CompileEnvironment &cenv) {
    std::atomic<bool> autotuner_result{false};
    std::atomic<bool> responder_ready{false};
    std::atomic<bool> request_received{false};
    std::stringstream dsl_code;
    for (const auto &file : cenv.dsl_files) {
      std::ifstream in(file);
      if (!in.is_open()) {
        std::cerr << "Failed to open DSL file: " << file << '\n';
        return false;
      }
      dsl_code << in.rdbuf() << "\n";
    }

    auto file_path = write_dsl_file(dsl_code.str());
    auto program = compile_dsl(file_path);
    if (!program) {
      return false;
    }
    // Fill database
    database::SnapshotManager snapm(db_);
    if (cenv.globals.has_value()) {
      snapm.import_from_json(cenv.globals.value());
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
            falcon_core::physics::config::Loader loader(cenv.device_config);
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
        autotuner_result = run_autotuner(*program, cenv.autotuner_name,
                                         cenv.params, cenv.expect_success);
      } catch (const std::exception &e) {
        std::cout << "EXCEPTION in client thread: " << e.what() << '\n';
        throw;
      }
    });

    client.join();
    responder.join();

    return autotuner_result;
  }
  void export_snapshot(const std::filesystem::path &path) {
    database::SnapshotManager snapm(db_);
    snapm.export_to_json(path);
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
