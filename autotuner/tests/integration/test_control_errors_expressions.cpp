// autotuner/tests/dsl/test_integration.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class IntegrationTest : public DSLTestBase {};

TEST_F(IntegrationTest, SimpleSweep) {
  const char *dsl = R"(
autotuner SimpleSweep (float start, float end, float step) [] -> (int count, float final_value) {
  params {
    float current = 0.0;
    int count = 0;
    float final_value = 0.0;
  }
  
  start -> init;
  
  state init {
    current = start;
    -> sweep;
  }
  
  state sweep {
    count = count + 1;
    final_value = current;
    current = current + step;
    
    if (current <= end) -> sweep;
    else -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("start", 0.0);
  params.set("end", 1.0);
  params.set("step", 0.1);

  ASSERT_TRUE(compile_and_run(dsl, "SimpleSweep", params));

  EXPECT_EQ(params.get<int64_t>("count"), 11);
  EXPECT_NEAR(params.get<double>("final_value"), 1.0, 0.01);
}

TEST_F(IntegrationTest, MultiStageWorkflow) {
  const char *dsl = R"(
autotuner MultiStageWorkflow () [] -> (string stage, bool initialized) {
  params {
    bool initialized = false;
    string stage = "none";
  }
  
  start -> initialize;
  
  state initialize {
    initialized = true;
    stage = "init";
    -> process;
  }
  
  state process {
    if (initialized == true) {
      stage = "processing";
      -> validate;
    }
    else -> error_state;
  }
  
  state validate {
    if (stage == "processing") {
      stage = "validated";
      -> finalize;
    }
    else -> error_state;
  }
  
  state finalize {
    stage = "completed";
    -> done;
  }
  
  state done {
    terminal;
  }
  
  state error_state {
    stage = "error";
    terminal;
  }
}
)";

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(dsl, "MultiStageWorkflow", params));

  EXPECT_TRUE(params.get<bool>("initialized"));
  EXPECT_EQ(params.get<std::string>("stage"), "completed");
}

TEST_F(IntegrationTest, GenericAutotuner) {
  const char *dsl = R"(
autotuner GenericProcessor (float input) [device_id] -> (float output) {
  params {
    float output = 0.0;
    float multiplier = 2.0;
  }
  
  start -> process;
  
  state process {
    output = input * multiplier;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("input", 10.0);
  params.set("device_id", std::string("dev1")); // Generic parameter

  ASSERT_TRUE(compile_and_run(dsl, "GenericProcessor", params));

  EXPECT_DOUBLE_EQ(params.get<double>("output"), 20.0);
}
