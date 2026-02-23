// autotuner/tests/dsl/test_control_flow.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class ControlFlowTest : public DSLTestBase {};

TEST_F(ControlFlowTest, SequentialStates) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/control_flow/sequential-test.fal"),
      "SequentialTest", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<int64_t>("step"), 3);
}

TEST_F(ControlFlowTest, LoopLikeIteration) {
  ParameterMap params;
  params.set("max_iterations", static_cast<int64_t>(10));
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/control_flow/iteration-test.fal"),
      "IterationTest", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<int64_t>("counter"), 10);
}

TEST_F(ControlFlowTest, ConditionalChainNegative) {
  ParameterMap params;
  params.set("value", static_cast<int64_t>(-5));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<std::string>("category"), "negative");
}

TEST_F(ControlFlowTest, ConditionalChainingZero) {
  ParameterMap params;
  params.set("value", static_cast<int64_t>(0));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<std::string>("category"), "zero");
}

TEST_F(ControlFlowTest, ConditionalChainingSmall) {
  ParameterMap params;
  params.set("value", static_cast<int64_t>(5));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<std::string>("category"), "small");
}

TEST_F(ControlFlowTest, ConditionalChainingMedium) {
  ParameterMap params;
  params.set("value", static_cast<int64_t>(50));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<std::string>("category"), "medium");
}

TEST_F(ControlFlowTest, ConditionalChainingLarge) {
  ParameterMap params;
  params.set("value", static_cast<int64_t>(200));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<std::string>("category"), "large");
}

TEST_F(ControlFlowTest, SimpleSweep) {

  ParameterMap params;
  params.set("start", 0.0);
  params.set("end", 1.0);
  params.set("step", 0.1);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/control_flow/simple-sweep.fal"),
      "SimpleSweep", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<int64_t>("count"), 11);
  EXPECT_NEAR(params.get<double>("final_value"), 1.0, 0.01);
}

TEST_F(ControlFlowTest, MultiStageWorkflow) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/multi-stage-workflow.fal"),
      "MultiStageWorkflow", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_TRUE(params.get<bool>("initialized"));
  EXPECT_EQ(params.get<std::string>("stage"), "completed");
}
