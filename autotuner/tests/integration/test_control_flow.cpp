// autotuner/tests/dsl/test_control_flow.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class ControlFlowTest : public DSLTestBase {};

TEST_F(ControlFlowTest, SequentialStates) {
  const char *dsl = R"(
autotuner SequentialTest () [] -> (int step) {
  params {
    int step = 0;
  }
  
  start -> state1;
  
  state state1 {
    step = 1;
    -> state2;
  }
  
  state state2 {
    step = 2;
    -> state3;
  }
  
  state state3 {
    step = 3;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(dsl, "SequentialTest", params));

  EXPECT_EQ(params.get<int64_t>("step"), 3);
}

TEST_F(ControlFlowTest, LoopLikeIteration) {
  const char *dsl = R"(
autotuner IterationTest (int max_iterations) [] -> (int counter) {
  params {
    int counter = 0;
  }
  
  start -> loop;
  
  state loop {
    counter = counter + 1;
    
    if (counter < max_iterations) -> loop;
    else -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("max_iterations", static_cast<int64_t>(10));

  ASSERT_TRUE(compile_and_run(dsl, "IterationTest", params));

  EXPECT_EQ(params.get<int64_t>("counter"), 10);
}

TEST_F(ControlFlowTest, ConditionalChaining) {
  const char *dsl = R"(
autotuner ConditionalChain (int value) [] -> (string category) {
  params {
    string category = "";
  }
  
  start -> classify;
  
  state classify {
    if (value < 0) -> negative;
    else if (value == 0) -> zero;
    else if (value < 10) -> small;
    else if (value < 100) -> medium;
    else -> large;
  }
  
  state negative {
    category = "negative";
    terminal;
  }
  
  state zero {
    category = "zero";
    terminal;
  }
  
  state small {
    category = "small";
    terminal;
  }
  
  state medium {
    category = "medium";
    terminal;
  }
  
  state large {
    category = "large";
    terminal;
  }
}
)";

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
