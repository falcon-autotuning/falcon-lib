#include "dsl_test_base.hpp"
#include <gtest/gtest.h>
using namespace falcon::dsl;
using namespace falcon::dsl::test;
using namespace falcon::typing;

class DatabaseAccessTest : public DSLTestBase {};

TEST_F(DatabaseAccessTest, StateRead0) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/database_access/state-read.fal"),
      "StateRead", params, true,
      std::filesystem::path(
          "test-autotuners/database_access/state-read-0.json")};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_FALSE(std::get<bool>(outputs[0]));
}

TEST_F(DatabaseAccessTest, StateRead1) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/database_access/state-read.fal"),
      "StateRead", params, true,
      std::filesystem::path(
          "test-autotuners/database_access/state-read-1.json")};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_TRUE(std::get<bool>(outputs[0]));
}

TEST_F(DatabaseAccessTest, StateWrite) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/database_access/state-write.fal"),
      "StateWrite", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  auto out_path = std::filesystem::temp_directory_path() / "state-write.json";
  export_snapshot(out_path);
  std::ifstream in(out_path);
  ASSERT_TRUE(in.is_open()) << "Could not open output file";
  std::string contents((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
  in.close();
  auto json = nlohmann::json::parse(contents);
  ASSERT_EQ(json.size(), 1);
  EXPECT_EQ(json[0]["scope"], "globals");
  EXPECT_EQ(json[0]["name"], "completed");
  EXPECT_EQ(json[0]["extra"], "default");
  EXPECT_EQ(json[0]["characteristic"], false);
}

TEST_F(DatabaseAccessTest, TransitionRead) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/database_access/transition-read.fal"),
      "TransitionRead", params, true,
      std::filesystem::path(
          "test-autotuners/database_access/transition-read.json")};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_TRUE(std::get<bool>(outputs[0]));
}

TEST_F(DatabaseAccessTest, TransitionWrite) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/database_access/transition-write.fal"),
      "TransitionWrite", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  auto out_path =
      std::filesystem::temp_directory_path() / "transition-write.json";
  export_snapshot(out_path);
  std::ifstream in(out_path);
  ASSERT_TRUE(in.is_open()) << "Could not open output file";
  std::string contents((std::istreambuf_iterator<char>(in)),
                       std::istreambuf_iterator<char>());
  in.close();
  auto json = nlohmann::json::parse(contents);
  ASSERT_EQ(json.size(), 1);
  EXPECT_EQ(json[0]["scope"], "device");
  EXPECT_EQ(json[0]["name"], "completed");
  EXPECT_EQ(json[0]["extra"], "default");
  EXPECT_EQ(json[0]["characteristic"], true);
}
