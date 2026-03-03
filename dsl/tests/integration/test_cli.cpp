#include "dsl_test_base.hpp"
#include "falcon-dsl/log.hpp"
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fmt/format.h>
#include <gtest/gtest.h>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

using namespace falcon::dsl::test;

class CLIProcessTest : public DSLTestBase {
protected:
  // Helper to run CLI and capture output
  static int run_cli(const std::vector<std::string> &args, std::string &output,
                     std::string &error) {
    std::string cwd = std::filesystem::current_path().string();
    falcon::dsl::log::info(fmt::format("Current working directory: {}", cwd));
    std::string cli_path = "../falcon-run";
    if (!std::filesystem::exists(cli_path)) {
      falcon::dsl::log::error(
          fmt::format("falcon-run not found at {}", cli_path));
      return 127;
    }
    std::ostringstream cmd;
    cmd << cli_path;
    for (const auto &arg : args) {
      cmd << " " << arg;
    }
    cmd << " 2>&1";
    std::string command_str = cmd.str();
    falcon::dsl::log::info(fmt::format("Running command: {}", command_str));
    FILE *pipe = popen(command_str.c_str(), "r");
    if (pipe == nullptr) {
      error = "Failed to open pipe for command: " + command_str;
      return 127;
    }
    char buffer[256];
    output.clear();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
      output += buffer;
    int status = pclose(pipe);
    return WEXITSTATUS(status);
  }
};

TEST_F(CLIProcessTest, SimpleBoolAutotunerRuns) {
  std::filesystem::path fal_file =
      std::filesystem::path(__FILE__).parent_path().parent_path() /
      "test-autotuners/simple_bool/simple_bool.fal";
  falcon::dsl::log::info(
      fmt::format("The current fal_file is at {}", fal_file.c_str()));
  falcon::typing::ParameterMap params;
  SingleCompileEnvironment cenv{fal_file, "SimpleBool", params, true};
  std::string output;
  std::string error;
  with_test_environment(cenv, [&]() {
    int exit_code = run_cli({"SimpleBool", fal_file.string()}, output, error);
    if (exit_code != 0) {
      std::cerr << "CLI output:\n" << output << '\n';
      std::cerr << "CLI error:\n" << error << '\n';
    }
    ASSERT_EQ(exit_code, 0);
    ASSERT_NE(output.find("Autotuner 'SimpleBool' completed."),
              std::string::npos);
    ASSERT_NE(output.find("true"), std::string::npos);
  });
}

TEST_F(CLIProcessTest, ListAutotuners) {
  std::filesystem::path fal_file =
      std::filesystem::path(__FILE__).parent_path().parent_path() /
      "test-autotuners/simple_bool/simple_bool.fal";
  falcon::typing::ParameterMap params;
  SingleCompileEnvironment cenv{fal_file, "SimpleBool", params, true};
  std::string output;
  std::string error;
  with_test_environment(cenv, [&]() {
    int exit_code = run_cli({"--list", fal_file.string()}, output, error);

    ASSERT_EQ(exit_code, 0);
    ASSERT_NE(output.find("Loaded autotuners:"), std::string::npos);
    ASSERT_NE(output.find("SimpleBool"), std::string::npos);
  });
}
