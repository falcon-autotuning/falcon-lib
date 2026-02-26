
#include "dsl_test_base.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class NestTest : public DSLTestBase {};

TEST_F(NestTest, SimpleNest) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/simple-nest.fal"),
      "ErrorViewer", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_GE(outputs.size(), 2);
  EXPECT_TRUE(std::get<bool>(outputs[0]));  // outer_completed
  EXPECT_FALSE(std::get<bool>(outputs[1])); // inner_completed
  // TODO: capture error message
}

TEST_F(NestTest, ConditionalNestGood) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.emplace("a", static_cast<int64_t>(5));
  params.emplace("b", static_cast<int64_t>(0));
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_GE(outputs.size(), 2);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), static_cast<int64_t>(0));
  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(outputs[1]));
}

TEST_F(NestTest, ConditionalNestBadAdd) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.emplace("a", static_cast<int64_t>(0));
  params.emplace("b", static_cast<int64_t>(5));
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_GE(outputs.size(), 2);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), static_cast<int64_t>(0));
  EXPECT_EQ(std::get<ErrorObject>(outputs[1]).message,
            "Cannot add a less than b");
  EXPECT_FALSE(std::get<ErrorObject>(outputs[1]).is_fatal);
}

TEST_F(NestTest, ConditionalNestBadMult) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/nest/conditional-nest.fal"),
      "ConditionalNest", params, true};
  params.emplace("a", static_cast<int64_t>(-2));
  params.emplace("b", static_cast<int64_t>(5));
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_GE(outputs.size(), 2);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), static_cast<int64_t>(0));
  EXPECT_EQ(std::get<ErrorObject>(outputs[1]).message,
            "Cannot add a less than b");
  EXPECT_FALSE(std::get<ErrorObject>(outputs[1]).is_fatal);
}
