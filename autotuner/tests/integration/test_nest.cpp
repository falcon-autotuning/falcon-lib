
#include "dsl_test_base.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class NestTest : public DSLTestBase {};

TEST_F(NestTest, SimpleNest) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/simple-nest.fal"),
      "SimpleNest", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<bool>("outer_completed"), true);
  EXPECT_EQ(params.get<bool>("inner_completed"), false);
  // TODO: capture error message
}

TEST_F(NestTest, ConditionalNestGood) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.set("a", static_cast<int64_t>(5));
  params.set("b", static_cast<int64_t>(0));
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<bool>("outer_completed"), true);
  EXPECT_EQ(params.get<bool>("inner_completed"), false);
  EXPECT_EQ(params.get<int64_t>("out"), static_cast<int64_t>(25));
}
TEST_F(NestTest, ConditionalNestBadAdd) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.set("a", static_cast<int64_t>(0));
  params.set("b", static_cast<int64_t>(5));
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<bool>("outer_completed"), true);
  EXPECT_EQ(params.get<bool>("inner_completed"), false);
  EXPECT_EQ(params.get<int64_t>("out"), static_cast<int64_t>(0));
  // TODO: capture error message
}
TEST_F(NestTest, ConditionalNestBadMult) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.set("a", static_cast<int64_t>(-2));
  params.set("b", static_cast<int64_t>(5));
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_EQ(params.get<bool>("outer_completed"), true);
  EXPECT_EQ(params.get<bool>("inner_completed"), false);
  EXPECT_EQ(params.get<int64_t>("out"), static_cast<int64_t>(0));
  // TODO: capture error message
}
