#include "dsl_test_base.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class ExpressionsTest : public DSLTestBase {};

TEST_F(ExpressionsTest, ArithmeticOperations) {
  ParameterMap params;
  params["x"] = 10.0;
  params["y"] = 3.0;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/arithmetic-test.fal"),
      "ArithmeticTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 4);
  EXPECT_DOUBLE_EQ(std::get<double>(outputs[0]), 13.0);   // add
  EXPECT_DOUBLE_EQ(std::get<double>(outputs[1]), 7.0);    // sub
  EXPECT_DOUBLE_EQ(std::get<double>(outputs[2]), 30.0);   // mul
  EXPECT_NEAR(std::get<double>(outputs[3]), 3.333, 0.01); // div
}

TEST_F(ExpressionsTest, ComparisonOperators) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(20);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/comparison-test.fal"),
      "ComparisonTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 5);
  EXPECT_FALSE(std::get<bool>(outputs[0])); // gt
  EXPECT_TRUE(std::get<bool>(outputs[1]));  // lt
  EXPECT_FALSE(std::get<bool>(outputs[2])); // eq
  EXPECT_FALSE(std::get<bool>(outputs[3])); // gte
  EXPECT_TRUE(std::get<bool>(outputs[4]));  // lte
}

TEST_F(ExpressionsTest, LogicalOperations) {
  ParameterMap params;
  params["flag1"] = true;
  params["flag2"] = false;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/logical-test.fal"),
      "LogicalTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 1);
  EXPECT_EQ(std::get<std::string>(outputs[0]), "expected");
}

TEST_F(ExpressionsTest, UnaryOperators) {
  ParameterMap params;
  params["value"] = static_cast<int64_t>(42);
  params["flag"] = true;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/unary-test.fal"),
      "UnaryTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 2);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), -42); // negated
  EXPECT_FALSE(std::get<bool>(outputs[1]));      // inverted
}

TEST_F(ExpressionsTest, ComplexExpressions) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(5);
  params["b"] = static_cast<int64_t>(3);
  params["c"] = static_cast<int64_t>(4);
  SingleCompileEnvironment cenv{
      std::filesystem::path(
          "test-autotuners/expressions/complex-expr-test.fal"),
      "ComplexExprTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 1);
  // (5 + 3) * 4 - 10 = 8 * 4 - 10 = 32 - 10 = 22
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 22);
}
