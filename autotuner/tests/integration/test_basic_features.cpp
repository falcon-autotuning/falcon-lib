// autotuner/tests/dsl/test_basic_features.cpp
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
  EXPECT_TRUE(params.has("completed"));
  EXPECT_TRUE(params.get<bool>("completed"));
}

TEST_F(BasicFeaturesTest, SimpleTransition) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/simple-transition.fal"),
      "SimpleTransition", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_TRUE(params.has("completed"));
  EXPECT_TRUE(params.get<bool>("completed"));
}

TEST_F(BasicFeaturesTest, ConditionalBranchingLow) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/conditional-branch.fal"),
      "ConditionalBranch", params, true};
  params.set("threshold", static_cast<int64_t>(40));
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<std::string>("result"), "low");
}

TEST_F(BasicFeaturesTest, ConditionalBranchingHigh) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/conditional-branch.fal"),
      "ConditionalBranch", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  params.set("threshold", static_cast<int64_t>(60));
  EXPECT_EQ(params.get<std::string>("result"), "high");
}

TEST_F(BasicFeaturesTest, ParameterInputOutput) {
  ParameterMap params;
  params.set("a", static_cast<int64_t>(10));
  params.set("b", static_cast<int64_t>(5));

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/calculator.fal"),
      "Calculator", params, true};

  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<int64_t>("sum"), 15);
  EXPECT_EQ(params.get<int64_t>("product"), 50);
}

TEST_F(BasicFeaturesTest, TempVariables) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/temp-vars.fal"),
      "TempVars", params, true};

  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<int64_t>("final_value"), 42);
  EXPECT_FALSE(params.has("intermediate")); // Temp var should not persist
}

TEST_F(BasicFeaturesTest, TerminalState) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/basic_features/terminal.fal"),
      "Terminal", params, true};

  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<int64_t>("steps"), 2);
}

TEST_F(BasicFeaturesTest, GenericAutotuner) {

  ParameterMap params;
  params.set("input", 10.0);
  params.set("device_id", std::string("dev1")); // Generic parameter

  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/basic_features/generic-processor.fal"),
      "GenericProcessor", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_DOUBLE_EQ(params.get<double>("output"), 20.0);
}
