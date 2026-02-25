#include "dsl_test_base.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class BasicFeaturesTest : public DSLTestBase {};

TEST_F(BasicFeaturesTest, TheSimplest) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/no-transition.fal"),
      "NoTransition", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_TRUE(std::get<bool>(outputs[0]));
}

TEST_F(BasicFeaturesTest, SimpleTransition) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/simple-transition.fal"),
      "SimpleTransition", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_TRUE(std::get<bool>(outputs[0]));
}

TEST_F(BasicFeaturesTest, ConditionalBranchingHigh) {
  ParameterMap params;
  params["threshold"] = static_cast<int64_t>(40);
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/conditional-branch.fal"),
      "ConditionalBranch", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_EQ(std::get<std::string>(outputs[0]), "high");
}

TEST_F(BasicFeaturesTest, ConditionalBranchingLow) {
  ParameterMap params;
  params["threshold"] = static_cast<int64_t>(60);
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/conditional-branch.fal"),
      "ConditionalBranch", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_EQ(std::get<std::string>(outputs[0]), "low");
}

TEST_F(BasicFeaturesTest, ParameterInputOutput) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(5);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/calculator.fal"),
      "Calculator", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 2);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 15);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 50);
}

TEST_F(BasicFeaturesTest, TempVariables) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/temp-vars.fal"),
      "TempVars", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 42);
  // No intermediate variable in outputs vector
}

TEST_F(BasicFeaturesTest, TerminalState) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/terminal.fal"),
      "Terminal", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 2);
}
