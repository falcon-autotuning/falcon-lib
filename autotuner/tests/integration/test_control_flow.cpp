// autotuner/tests/dsl/test_control_flow.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class ControlFlowTest : public DSLTestBase {};

TEST_F(ControlFlowTest, SequentialStates) {

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/sequential-test.fal"),
      "SequentialTest", params));

  EXPECT_EQ(params.get<int64_t>("step"), 3);
}

TEST_F(ControlFlowTest, LoopLikeIteration) {
  ParameterMap params;
  params.set("max_iterations", static_cast<int64_t>(10));

  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/iteration-test.fal"),
      "IterationTest", params));

  EXPECT_EQ(params.get<int64_t>("counter"), 10);
}

TEST_F(ControlFlowTest, ConditionalChaining) {
  auto dsl = std::filesystem::path("test-autotuners/conditional-chain.fal");
  // Test various categories
  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(-5));
    ASSERT_TRUE(compile_and_run(dsl, "ConditionalChain", params));
    EXPECT_EQ(params.get<std::string>("category"), "negative");
  }

  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(0));
    ASSERT_TRUE(compile_and_run(dsl, "ConditionalChain", params));
    EXPECT_EQ(params.get<std::string>("category"), "zero");
  }

  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(5));
    ASSERT_TRUE(compile_and_run(dsl, "ConditionalChain", params));
    EXPECT_EQ(params.get<std::string>("category"), "small");
  }

  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(50));
    ASSERT_TRUE(compile_and_run(dsl, "ConditionalChain", params));
    EXPECT_EQ(params.get<std::string>("category"), "medium");
  }

  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(200));
    ASSERT_TRUE(compile_and_run(dsl, "ConditionalChain", params));
    EXPECT_EQ(params.get<std::string>("category"), "large");
  }
}
