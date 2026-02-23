#include "dsl_test_base.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class DatabaseAccessTest : public DSLTestBase {};

TEST_F(DatabaseAccessTest, StateRead0) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/database_access/state-read.fal"),
      "StateRead", params, true,
      std::filesystem::path(
          "test-autotuners/database_access/state-read-0.json")};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<bool>("completed"), false);
}

TEST_F(DatabaseAccessTest, StateRead1) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/database_access/state-read.fal"),
      "StateRead", params, true,
      std::filesystem::path(
          "test-autotuners/database_access/state-read-1.json")};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<bool>("completed"), true);
}

TEST_F(DatabaseAccessTest, StateWrite) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/database_access/state-write.fal"),
      "StateWrite", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
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
  EXPECT_EQ(json[0]["value"], false);
}

TEST_F(DatabaseAccessTest, TransitionRead) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/database_access/transition-read.fal"),
      "TransitionRead", params, true,
      std::filesystem::path(
          "test-autotuners/database_access/transition-read.json")};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<bool>("completed"), true);
}

TEST_F(DatabaseAccessTest, TransitionWrite) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/database_access/transition-write.fal"),
      "TransitionWrite", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
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
  EXPECT_EQ(json[0]["scope"], "globals");
  EXPECT_EQ(json[0]["name"], "completed");
  EXPECT_EQ(json[0]["extra"], "default");
  EXPECT_EQ(json[0]["value"], false);
}
