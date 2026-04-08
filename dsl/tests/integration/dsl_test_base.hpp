#pragma once

#include "falcon-dsl/AutotunerEngine.hpp"
#include "falcon-dsl/log.hpp"
#include <atomic>
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
  std::vector<std::filesystem::path> routine_libs;

  CompileEnvironment(
      std::vector<std::filesystem::path> dsl_files_,
      std::string autotuner_name_, typing::ParameterMap &params_,
      bool expect_success_ = true,
      std::optional<std::filesystem::path> globals_ = std::nullopt,
      std::vector<std::filesystem::path> routine_libs_ = {})
      : dsl_files(std::move(dsl_files_)),
        autotuner_name(std::move(autotuner_name_)), params(params_),
        expect_success(expect_success_), globals(std::move(globals_)),
        routine_libs(std::move(routine_libs_)) {}
};

struct SingleCompileEnvironment : public CompileEnvironment {
  SingleCompileEnvironment(
      std::filesystem::path dsl_file_, std::string autotuner_name_,
      typing::ParameterMap &params_, bool expect_success_ = true,
      std::optional<std::filesystem::path> globals_ = std::nullopt,
      std::vector<std::filesystem::path> routine_libs_ = {})
      : CompileEnvironment({std::move(dsl_file_)}, std::move(autotuner_name_),
                           params_, expect_success_, std::move(globals_),
                           std::move(routine_libs_)) {}
};

class RoutineTestFixture : public ::testing::Test {
protected:
  void SetUp() override {
    setupEnvironment();
    log::debug("environment variables set");
  }
  void TearDown() override { unsetenv("NATS_URL"); }

  void setupEnvironment() {
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

class DSLTestBase : public RoutineTestFixture {
protected:
  void SetUp() override {
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
    client_func();

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

  std::filesystem::path test_dir_;
};

} // namespace falcon::dsl::test
