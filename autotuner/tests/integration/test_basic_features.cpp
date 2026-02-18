// autotuner/tests/dsl/test_basic_features.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class BasicFeaturesTest : public DSLTestBase {};

TEST_F(BasicFeaturesTest, TheSimplest) {
  const char *dsl = R"(
autotuner NoTransition () -> (bool completed) {
  params {
    bool completed = false;
  }
  
  start -> done;
  
  state done {
    completed = true;
    terminal;
  }
  
}
)";

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(dsl, "NoTransition", params));

  EXPECT_TRUE(params.has("completed"));
  EXPECT_TRUE(params.get<bool>("completed"));
}

TEST_F(BasicFeaturesTest, SimpleTransition) {
  const char *dsl = R"(
autotuner SimpleTransition () -> (bool completed) {
  params {
    bool completed = false;
  }
  
  start -> init;
  
  state init {
    completed = true;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(dsl, "SimpleTransition", params));

  EXPECT_TRUE(params.has("completed"));
  EXPECT_TRUE(params.get<bool>("completed"));
}

TEST_F(BasicFeaturesTest, ConditionalBranching) {
  const char *dsl = R"(
autotuner ConditionalBranch (int threshold) -> (string result) {
  params {
    int value = 50;
    string result = "";
  }
  
  start -> check;
  
  state check {
    if (value > threshold) -> high;
    else -> low;
  }
  
  state high {
    result = "high";
    -> done;
  }
  
  state low {
    result = "low";
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  // Test high branch
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
  const char *dsl = R"(
autotuner Calculator (int a, int b) -> (int sum, int product) {
  params {
    int sum = 0;
    int product = 0;
  }
  
  start -> calculate;
  
  state calculate {
    sum = a + b;
    product = a * b;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("a", static_cast<int64_t>(10));
  params.set("b", static_cast<int64_t>(5));

  ASSERT_TRUE(compile_and_run(dsl, "Calculator", params));

  EXPECT_EQ(params.get<int64_t>("sum"), 15);
  EXPECT_EQ(params.get<int64_t>("product"), 50);
}

TEST_F(BasicFeaturesTest, TempVariables) {
  const char *dsl = R"(
autotuner TempVarTest () [] -> (int final_value) {
  params {
    int final_value = 0;
  }
  
  start -> compute;
  
  state compute {
    temp {
      int intermediate = 0;
    }
    
    intermediate = 42;
    final_value = intermediate;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(dsl, "TempVarTest", params));

  EXPECT_EQ(params.get<int64_t>("final_value"), 42);
  EXPECT_FALSE(params.has("intermediate")); // Temp var should not persist
}

TEST_F(BasicFeaturesTest, TerminalState) {
  const char *dsl = R"(
autotuner TerminalTest () [] -> (int steps) {
  params {
    int steps = 0;
  }
  
  start -> step1;
  
  state step1 {
    steps = 1;
    -> step2;
  }
  
  state step2 {
    steps = 2;
    -> finish;
  }
  
  state finish {
    terminal;
  }
}
)";

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(dsl, "TerminalTest", params));

  EXPECT_EQ(params.get<int64_t>("steps"), 2);
}
