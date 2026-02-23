
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
  EXPECT_TRUE(std::get<bool>(params.at("outer_completed")));
  EXPECT_FALSE(std::get<bool>(params.at("inner_completed")));
  // TODO: capture error message
}

TEST_F(NestTest, ConditionalNestGood) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.emplace("a", static_cast<int64_t>(5));
  params.emplace("b", static_cast<int64_t>(0));
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_TRUE(std::get<bool>(params.at("outer_completed")));
  EXPECT_TRUE(std::get<bool>(params.at("inner_completed")));
  EXPECT_EQ(std::get<int64_t>(params.at("inner_completed")),
            static_cast<int64_t>(25));
}
TEST_F(NestTest, ConditionalNestBadAdd) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.emplace("a", static_cast<int64_t>(0));
  params.emplace("b", static_cast<int64_t>(5));
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_FALSE(std::get<bool>(params.at("outer_completed")));
  EXPECT_FALSE(std::get<bool>(params.at("inner_completed")));
  EXPECT_EQ(std::get<int64_t>(params.at("inner_completed")),
            static_cast<int64_t>(0));
  // TODO: capture error message
}
TEST_F(NestTest, ConditionalNestBadMult) {

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.emplace("a", static_cast<int64_t>(-2));
  params.emplace("b", static_cast<int64_t>(5));
  ASSERT_TRUE(compile_and_run(cenv));
  EXPECT_TRUE(std::get<bool>(params.at("outer_completed")));
  EXPECT_FALSE(std::get<bool>(params.at("inner_completed")));
  EXPECT_EQ(std::get<int64_t>(params.at("inner_completed")),
            static_cast<int64_t>(0));
  // TODO: capture error message
}
