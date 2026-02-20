// autotuner/tests/dsl/test_integration.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class IntegrationTest : public DSLTestBase {};

TEST_F(IntegrationTest, SimpleSweep) {

  ParameterMap params;
  params.set("start", 0.0);
  params.set("end", 1.0);
  params.set("step", 0.1);

  ASSERT_TRUE(
      compile_and_run(std::filesystem::path("test-autotuners/simple-sweep.fal"),
                      "SimpleSweep", params));

  EXPECT_EQ(params.get<int64_t>("count"), 11);
  EXPECT_NEAR(params.get<double>("final_value"), 1.0, 0.01);
}

TEST_F(IntegrationTest, MultiStageWorkflow) {

  ParameterMap params;
  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/multi-stage-workflow.fal"),
      "MultiStageWorkflow", params));

  EXPECT_TRUE(params.get<bool>("initialized"));
  EXPECT_EQ(params.get<std::string>("stage"), "completed");
}

TEST_F(IntegrationTest, GenericAutotuner) {

  ParameterMap params;
  params.set("input", 10.0);
  params.set("device_id", std::string("dev1")); // Generic parameter

  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/generic-processor.fal"),
      "GenericProcessor", params));

  EXPECT_DOUBLE_EQ(params.get<double>("output"), 20.0);
}
