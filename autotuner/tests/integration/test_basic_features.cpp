// autotuner/tests/dsl/test_basic_features.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class BasicFeaturesTest : public DSLTestBase {};

TEST_F(BasicFeaturesTest, TheSimplest) {
  ParameterMap params;
  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/no-transition.fal"),
      "NoTransition", params));
  EXPECT_TRUE(params.has("completed"));
  EXPECT_TRUE(params.get<bool>("completed"));
}

TEST_F(BasicFeaturesTest, SimpleTransition) {
  ParameterMap params;
  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/simple-transition.fal"),
      "SimpleTransition", params));

  EXPECT_TRUE(params.has("completed"));
  EXPECT_TRUE(params.get<bool>("completed"));
}

TEST_F(BasicFeaturesTest, ConditionalBranching) {
  auto dsl = std::filesystem::path("test-autotuners/simple-transition.fal");
  {
    ParameterMap params;
    params.set("threshold", static_cast<int64_t>(30));
    ASSERT_TRUE(compile_and_run(dsl, "ConditionalBranch", params));
    EXPECT_EQ(params.get<std::string>("result"), "high");
  }

  // Test low branch
  {
    ParameterMap params;
    params.set("threshold", static_cast<int64_t>(60));
    ASSERT_TRUE(compile_and_run(dsl, "ConditionalBranch", params));
    EXPECT_EQ(params.get<std::string>("result"), "low");
  }
}

TEST_F(BasicFeaturesTest, ParameterInputOutput) {
  ParameterMap params;
  params.set("a", static_cast<int64_t>(10));
  params.set("b", static_cast<int64_t>(5));

  ASSERT_TRUE(
      compile_and_run(std::filesystem::path("test-autotuners/calculator.fal"),
                      "Calculator", params));

  EXPECT_EQ(params.get<int64_t>("sum"), 15);
  EXPECT_EQ(params.get<int64_t>("product"), 50);
}

TEST_F(BasicFeaturesTest, TempVariables) {
  ParameterMap params;
  ASSERT_TRUE(
      compile_and_run(std::filesystem::path("test-autotuners/temp-vars.fal"),
                      "TempVars", params));

  EXPECT_EQ(params.get<int64_t>("final_value"), 42);
  EXPECT_FALSE(params.has("intermediate")); // Temp var should not persist
}

TEST_F(BasicFeaturesTest, TerminalState) {

  ParameterMap params;
  ASSERT_TRUE(
      compile_and_run(std::filesystem::path("test-autotuners/terminal.fal"),
                      "Terminal", params));

  EXPECT_EQ(params.get<int64_t>("steps"), 2);
}
