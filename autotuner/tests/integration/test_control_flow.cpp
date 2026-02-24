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
  ASSERT_NE(params.find("step"), params.end());
  EXPECT_EQ(std::get<int64_t>(params.at("step")), 3);
}

TEST_F(ControlFlowTest, LoopLikeIteration) {
  ParameterMap params;
  params.emplace("max_iterations", static_cast<int64_t>(10));
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/control_flow/iteration-test.fal"),
      "IterationTest", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("counter"), params.end());
  EXPECT_EQ(std::get<int64_t>(params.at("counter")), 10);
}

TEST_F(ControlFlowTest, ConditionalChainNegative) {
  ParameterMap params;
  params.emplace("value", static_cast<int64_t>(-5));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("category"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("category")), "negative");
}

TEST_F(ControlFlowTest, ConditionalChainingZero) {
  ParameterMap params;
  params.emplace("value", static_cast<int64_t>(0));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("category"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("category")), "zero");
}

TEST_F(ControlFlowTest, ConditionalChainingSmall) {
  ParameterMap params;
  params.emplace("value", static_cast<int64_t>(5));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("category"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("category")), "small");
}

TEST_F(ControlFlowTest, ConditionalChainingMedium) {
  ParameterMap params;
  params.emplace("value", static_cast<int64_t>(50));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("category"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("category")), "medium");
}

TEST_F(ControlFlowTest, ConditionalChainingLarge) {
  ParameterMap params;
  params.emplace("value", static_cast<int64_t>(200));
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/conditional-chain.fal"),
      "ConditionalChain", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("category"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("category")), "large");
}

TEST_F(ControlFlowTest, SimpleSweep) {
  ParameterMap params;
  params.emplace("begin", 0.0);
  params.emplace("end", 1.0);
  params.emplace("step", 0.1);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/control_flow/simple-sweep.fal"),
      "SimpleSweep", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("count"), params.end());
  ASSERT_NE(params.find("final_value"), params.end());
  EXPECT_EQ(std::get<int64_t>(params.at("count")), 11);
  EXPECT_NEAR(std::get<double>(params.at("final_value")), 1.0, 0.01);
}

TEST_F(ControlFlowTest, MultiStageWorkflow) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/control_flow/multi-stage-workflow.fal"),
      "MultiStageWorkflow", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("initialized"), params.end());
  ASSERT_NE(params.find("stage"), params.end());
  EXPECT_TRUE(std::get<bool>(params.at("initialized")));
  EXPECT_EQ(std::get<std::string>(params.at("stage")), "completed");
}
