#pragma once

#include "falcon-database/SnapshotManager.hpp"
#include "falcon-dsl/AutotunerEngine.hpp"
#include "falcon-dsl/log.hpp"
#include <atomic>
#include <falcon-comms/natsManager.hpp>
#include <falcon-typing/PrimitiveTypes.hpp>
#include <falcon_core/communications/Time.hpp>
#include <falcon_core/physics/config/Loader.hpp>
#include <falcon_core/physics/config/core/Config.hpp>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include <string>
#include <thread>
#include <utility>

namespace falcon::dsl::test {

struct CompileEnvironment {
  std::vector<std::filesystem::path> dsl_files;
  std::string autotuner_name;
  typing::ParameterMap &params;
  bool expect_success;
  std::optional<std::filesystem::path> globals;
  std::filesystem::path device_config;
  std::vector<std::filesystem::path> routine_libs;

  CompileEnvironment(
      std::vector<std::filesystem::path> dsl_files_,
      std::string autotuner_name_, typing::ParameterMap &params_,
      bool expect_success_ = true,
      std::optional<std::filesystem::path> globals_ = std::nullopt,
      std::filesystem::path device_config_ =
          std::filesystem::path(__FILE__).parent_path().parent_path() /
          "device-configs/three-dot.yml",
      std::vector<std::filesystem::path> routine_libs_ = {})
      : dsl_files(std::move(dsl_files_)),
        autotuner_name(std::move(autotuner_name_)), params(params_),
        expect_success(expect_success_), globals(std::move(globals_)),
        device_config(std::move(device_config_)),
        routine_libs(std::move(routine_libs_)) {}
};

struct SingleCompileEnvironment : public CompileEnvironment {
  SingleCompileEnvironment(
      std::filesystem::path dsl_file_, std::string autotuner_name_,
      typing::ParameterMap &params_, bool expect_success_ = true,
      std::optional<std::filesystem::path> globals_ = std::nullopt,
      std::filesystem::path device_config_ =
          std::filesystem::path(__FILE__).parent_path().parent_path() /
          "device-configs/three-dot.yml",
      std::vector<std::filesystem::path> routine_libs_ = {})
      : CompileEnvironment({std::move(dsl_file_)}, std::move(autotuner_name_),
                           params_, expect_success_, std::move(globals_),
                           std::move(device_config_),
                           std::move(routine_libs_)) {}
};

class RoutineTestFixture : public ::testing::Test {
protected:
  void SetUp() override {
    setupEnvironment();
    log::debug("environment variables set");
  }
  void TearDown() override {
    unsetenv("FALCON_DATABASE_URL");
    unsetenv("NATS_URL");
  }

  void setupEnvironment() {
    const char *test_db_url = std::getenv("TEST_DATABASE_URL");
    if (test_db_url == nullptr) {
      db_url_ = "postgresql://falcon_test:falcon_test_password@127.0.0.1:5433/"
                "falcon_test";
    } else {
      db_url_ = test_db_url;
    }
    setenv("FALCON_DATABASE_URL", db_url_.c_str(), 1);

    const char *test_nats_url = std::getenv("TEST_NATS_URL");
    if (test_nats_url == nullptr) {
      nats_url_ = "nats://localhost:4222";
    } else {
      nats_url_ = test_nats_url;
    }
    setenv("NATS_URL", nats_url_.c_str(), 1);
    setenv("LOG_LEVEL", "debug", 1);
  }
  [[nodiscard]] std::string getDatabaseUrl() const { return db_url_; }
  [[nodiscard]] std::string getNatsUrl() const { return nats_url_; }
  std::string db_url_;
  std::string nats_url_;
};

class DatabaseTestFixture : public RoutineTestFixture {
protected:
  void SetUp() override {
    RoutineTestFixture::SetUp();
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

class DSLTestBase : public DatabaseTestFixture {
protected:
  void SetUp() override {
    DatabaseTestFixture::SetUp();
    test_dir_ = std::filesystem::temp_directory_path() / "falcon_dsl_tests";
    std::filesystem::create_directories(test_dir_);
  }
  void TearDown() override {
    if (std::filesystem::exists(test_dir_)) {
      std::filesystem::remove_all(test_dir_);
    }
  }
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

  template <typename Func>
  void with_test_environment(CompileEnvironment &cenv, Func &&func) {
    // Fill database
    database::SnapshotManager snapm(db_);
    log::debug("The snapshot manager is started");
    if (cenv.globals.has_value()) {
      snapm.import_from_json(cenv.globals.value().string(), true);
    }
    log::debug("Imported the snapshot from the json");
    std::atomic<bool> responder_ready{false};
    std::atomic<bool> client_done{false};

    // Start the device config responder thread (NATS)
    std::thread responder([&]() {
      auto &hub = comms::NatsManager::instance();
      hub.subscribe(
          "INSTRUMENTHUB.DEVICE_CONFIG_REQUEST",
          [&](const std::string & /*msg*/) {
            DeviceConfigResponse response;
            response.timestamp =
                (int)falcon_core::communications::Time().time();
            falcon_core::physics::config::Loader loader(cenv.device_config);
            const falcon_core::physics::config::core::ConfigSP config =
                loader.config();
            response.response = config->to_json_string();
            nlohmann::json json = response.to_json();
            hub.publish("FALCON.DEVICE_CONFIG_RESPONSE", json.dump());
          });
      responder_ready = true;
      while (!client_done) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      hub.unsubscribe("INSTRUMENTHUB.DEVICE_CONFIG_REQUEST");
    });

    while (!responder_ready) {
      std::this_thread::yield();
    }

    // Run the provided function (e.g., CLI subprocess)
    func();

    client_done = true;
    responder.join();
  }
  std::tuple<bool, std::vector<typing::RuntimeValue>>
  compile_and_run(CompileEnvironment &cenv) {
    std::atomic<bool> autotuner_result{false};
    std::atomic<bool> client_done{false};
    std::mutex result_mutex;
    std::vector<typing::RuntimeValue> autotuner_output;

    auto client_func = [&]() {
      try {
        falcon::dsl::AutotunerEngine engine;
        // Load all DSL files
        for (const auto &file : cenv.dsl_files) {
         bool loaded = engine.load_fal_file(file.string());
         if (cenv.expect_success) {
           EXPECT_TRUE(loaded) << "Failed to load: " << file;
         }
         if (!loaded && !cenv.expect_success) {
           // Parse/load failure is the expected outcome — stop loading and
           // let the test verify the false result without trying to run.
           autotuner_result = false;
           client_done = true;
           return;
         }
        }

        // Load routine libraries
        for (const auto &lib_path : cenv.routine_libs) {
          std::string routine_name = lib_path.stem().string();
          std::string namespace_name = "default";
          RoutineConfig routine{.name = routine_name,
                                .library_path = lib_path.string(),
                                .name_space = namespace_name};
          EXPECT_TRUE(engine.load_routine_library(routine))
              << "Failed to load routine: " << lib_path;
        }

        auto result = engine.run_autotuner(cenv.autotuner_name, cenv.params);
        {
          std::lock_guard<std::mutex> lock(result_mutex);
          autotuner_output = result;
        }
        autotuner_result = true;
      } catch (const std::exception &e) {
        std::cout << "EXCEPTION in client thread: " << e.what() << '\n';
        autotuner_result = false;
      }
      client_done = true;
    };

    with_test_environment(cenv, client_func);

    bool result = autotuner_result;
    std::vector<typing::RuntimeValue> output;
    {
      std::lock_guard<std::mutex> lock(result_mutex);
      output = autotuner_output;
    }

    if (cenv.expect_success) {
      EXPECT_TRUE(result) << "Autotuner execution failed: "
                          << cenv.autotuner_name;
    } else {
      EXPECT_FALSE(result) << "Autotuner should have failed: "
                           << cenv.autotuner_name;
    }
    return std::make_tuple(result, output);
  }

  void export_snapshot(const std::filesystem::path &path) {
    database::SnapshotManager snapm(db_);
    snapm.export_to_json(path);
  }

  std::filesystem::path test_dir_;
};

} // namespace falcon::dsl::test
