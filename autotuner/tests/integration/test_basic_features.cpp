#include "dsl_test_base.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class BasicFeaturesTest : public DSLTestBase {};

TEST_F(BasicFeaturesTest, TheSimplest) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/no-transition.fal"),
      "NoTransition", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("completed"), params.end());
  EXPECT_TRUE(std::get<bool>(params.at("completed")));
}

TEST_F(BasicFeaturesTest, SimpleTransition) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/simple-transition.fal"),
      "SimpleTransition", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("completed"), params.end());
  EXPECT_TRUE(std::get<bool>(params.at("completed")));
}

TEST_F(BasicFeaturesTest, ConditionalBranchingLow) {
  ParameterMap params;
  params["threshold"] = static_cast<int64_t>(40);
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/conditional-branch.fal"),
      "ConditionalBranch", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("result"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("result")), "low");
}

TEST_F(BasicFeaturesTest, ConditionalBranchingHigh) {
  ParameterMap params;
  params["threshold"] = static_cast<int64_t>(60);
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/conditional-branch.fal"),
      "ConditionalBranch", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("result"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("result")), "high");
}

TEST_F(BasicFeaturesTest, ParameterInputOutput) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(5);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/calculator.fal"),
      "Calculator", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("sum"), params.end());
  ASSERT_NE(params.find("product"), params.end());
  EXPECT_EQ(std::get<int64_t>(params.at("sum")), 15);
  EXPECT_EQ(std::get<int64_t>(params.at("product")), 50);
}

TEST_F(BasicFeaturesTest, TempVariables) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/temp-vars.fal"),
      "TempVars", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("final_value"), params.end());
  EXPECT_EQ(std::get<int64_t>(params.at("final_value")), 42);
  EXPECT_EQ(params.find("intermediate"),
            params.end()); // Temp var should not persist
}

TEST_F(BasicFeaturesTest, TerminalState) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/terminal.fal"),
      "Terminal", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("steps"), params.end());
  EXPECT_EQ(std::get<int64_t>(params.at("steps")), 2);
}
