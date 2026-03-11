#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

class CLIProcessTest : public ::testing::Test {
protected:
  // Helper to run CLI and capture output
  static int run_cli(const std::vector<std::string> &args, std::string &output,
                     std::string &error) {
    // falcon_db_integration_tests resides in build/release/tests or
    // build/debug/tests. the executable falcon-db-cli should be in
    // build/release or build/debug.
    std::string cli_path = "../falcon-db-cli";
    if (!std::filesystem::exists(cli_path)) {
      std::cerr << "falcon-db-cli not found at " << cli_path << '\n';
      // Attempt another common path if not found there
      cli_path = "../../falcon-db-cli";
      if (!std::filesystem::exists(cli_path)) {
        std::cerr << "falcon-db-cli not found at " << cli_path << '\n';
        return 127;
      }
    }

    std::ostringstream cmd;
    cmd << cli_path;
    for (const auto &arg : args) {
      cmd << " " << arg;
    }
    cmd << " 2>&1";
    std::string command_str = cmd.str();

    FILE *pipe = popen(command_str.c_str(), "r");
    if (pipe == nullptr) {
      error = "Failed to open pipe for command: " + command_str;
      return 127;
    }

    char buffer[256];
    output.clear();
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      output += buffer;
    }

    int status = pclose(pipe);
    return WEXITSTATUS(status);
  }
};

TEST_F(CLIProcessTest, HelpCommandOutput) {
  std::string output, error;
  int exit_code = run_cli({"help"}, output, error);
  ASSERT_EQ(exit_code, 0) << "Output: " << output << "\nError: " << error;
  ASSERT_NE(output.find("falcon-db-cli: A command-line interface"),
            std::string::npos);
  ASSERT_NE(output.find("Usage:"), std::string::npos);
}

TEST_F(CLIProcessTest, SchemaCommandOutput) {
  std::string output, error;
  int exit_code = run_cli({"schema"}, output, error);
  ASSERT_EQ(exit_code, 0) << "Output: " << output << "\nError: " << error;
  ASSERT_NE(
      output.find("\"$schema\": \"http://json-schema.org/draft-07/schema#\""),
      std::string::npos);
  ASSERT_NE(
      output.find("\"title\": \"Falcon Device Characteristics Snapshot\""),
      std::string::npos);
}

TEST_F(CLIProcessTest, InvalidCommandError) {
  std::string output, error;
  int exit_code = run_cli({"unknown_command"}, output, error);
  ASSERT_NE(exit_code, 0);
  ASSERT_NE(output.find("Unknown command: unknown_command"), std::string::npos);
}

TEST_F(CLIProcessTest, DBArgMissingValue) {
  std::string output, error;
  int exit_code = run_cli({"--db"}, output, error);
  ASSERT_NE(exit_code, 0);
  ASSERT_NE(output.find("Error: --db requires an argument."),
            std::string::npos);
}

TEST_F(CLIProcessTest, ValidateNonExistentFile) {
  std::string output, error;
  int exit_code =
      run_cli({"snapshot", "validate", "does_not_exist.json"}, output, error);
  // It should fail validation because the file cannot be opened.
  ASSERT_NE(exit_code, 0);
  ASSERT_NE(output.find("Snapshot validation failed: does_not_exist.json"),
            std::string::npos);
}
