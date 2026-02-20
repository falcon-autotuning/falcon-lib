// autotuner/tests/dsl/test_expressions.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class ExpressionsTest : public DSLTestBase {};

TEST_F(ExpressionsTest, ArithmeticOperations) {
  ParameterMap params;
  params.set("x", 10.0);
  params.set("y", 3.0);

  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/arithmetic-test.fal"),
      "ArithmeticTest", params));

  EXPECT_DOUBLE_EQ(params.get<double>("add"), 13.0);
  EXPECT_DOUBLE_EQ(params.get<double>("sub"), 7.0);
  EXPECT_DOUBLE_EQ(params.get<double>("mul"), 30.0);
  EXPECT_NEAR(params.get<double>("div"), 3.333, 0.01);
}

TEST_F(ExpressionsTest, ComparisonOperators) {

  ParameterMap params;
  params.set("a", static_cast<int64_t>(10));
  params.set("b", static_cast<int64_t>(20));

  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/comparison-test.fal"),
      "ComparisonTest", params));

  EXPECT_FALSE(params.get<bool>("gt"));
  EXPECT_TRUE(params.get<bool>("lt"));
  EXPECT_FALSE(params.get<bool>("eq"));
  EXPECT_FALSE(params.get<bool>("gte"));
  EXPECT_TRUE(params.get<bool>("lte"));
}

TEST_F(ExpressionsTest, LogicalOperations) {

  ParameterMap params;
  params.set("flag1", true);
  params.set("flag2", false);

  ASSERT_TRUE(
      compile_and_run(std::filesystem::path("test-autotuners/logical-test.fal"),
                      "LogicalTest", params));

  EXPECT_EQ(params.get<std::string>("result"), "expected");
}

TEST_F(ExpressionsTest, UnaryOperators) {

  ParameterMap params;
  params.set("value", static_cast<int64_t>(42));
  params.set("flag", true);

  ASSERT_TRUE(
      compile_and_run(std::filesystem::path("test-autotuners/unary-test.fal"),
                      "UnaryTest", params));

  EXPECT_EQ(params.get<int64_t>("negated"), -42);
  EXPECT_FALSE(params.get<bool>("inverted"));
}

TEST_F(ExpressionsTest, ComplexExpressions) {

  ParameterMap params;
  params.set("a", static_cast<int64_t>(5));
  params.set("b", static_cast<int64_t>(3));
  params.set("c", static_cast<int64_t>(4));

  ASSERT_TRUE(compile_and_run(
      std::filesystem::path("test-autotuners/complex-expr-test.fal"),
      "ComplexExprTest", params));

  // (5 + 3) * 4 - 10 = 8 * 4 - 10 = 32 - 10 = 22
  EXPECT_EQ(params.get<int64_t>("result"), 22);
}
