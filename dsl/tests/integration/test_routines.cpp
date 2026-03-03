#include "dsl_test_base.hpp"
using namespace falcon::dsl;
using namespace falcon::dsl::test;
using namespace falcon::typing;

class RoutineTest : public DSLTestBase {};

TEST_F(RoutineTest, ConditionalNestGood) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/routines/simple-routine.fal"),
      "ConditionalNest", params, true};
  params.emplace("a", static_cast<int64_t>(5));
  params.emplace("b", static_cast<int64_t>(0));
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_GE(outputs.size(), 1);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), static_cast<int64_t>(0));
}
