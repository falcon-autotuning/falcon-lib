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
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("add"), params.end());
  ASSERT_NE(params.find("sub"), params.end());
  ASSERT_NE(params.find("mul"), params.end());
  ASSERT_NE(params.find("div"), params.end());
  EXPECT_DOUBLE_EQ(std::get<double>(params.at("add")), 13.0);
  EXPECT_DOUBLE_EQ(std::get<double>(params.at("sub")), 7.0);
  EXPECT_DOUBLE_EQ(std::get<double>(params.at("mul")), 30.0);
  EXPECT_NEAR(std::get<double>(params.at("div")), 3.333, 0.01);
}

TEST_F(ExpressionsTest, ComparisonOperators) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(20);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/comparison-test.fal"),
      "ComparisonTest", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("gt"), params.end());
  ASSERT_NE(params.find("lt"), params.end());
  ASSERT_NE(params.find("eq"), params.end());
  ASSERT_NE(params.find("gte"), params.end());
  ASSERT_NE(params.find("lte"), params.end());
  EXPECT_FALSE(std::get<bool>(params.at("gt")));
  EXPECT_TRUE(std::get<bool>(params.at("lt")));
  EXPECT_FALSE(std::get<bool>(params.at("eq")));
  EXPECT_FALSE(std::get<bool>(params.at("gte")));
  EXPECT_TRUE(std::get<bool>(params.at("lte")));
}

TEST_F(ExpressionsTest, LogicalOperations) {
  ParameterMap params;
  params["flag1"] = true;
  params["flag2"] = false;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/logical-test.fal"),
      "LogicalTest", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("result"), params.end());
  EXPECT_EQ(std::get<std::string>(params.at("result")), "expected");
}

TEST_F(ExpressionsTest, UnaryOperators) {
  ParameterMap params;
  params["value"] = static_cast<int64_t>(42);
  params["flag"] = true;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/unary-test.fal"),
      "UnaryTest", params, true};
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("negated"), params.end());
  ASSERT_NE(params.find("inverted"), params.end());
  EXPECT_EQ(std::get<int64_t>(params.at("negated")), -42);
  EXPECT_FALSE(std::get<bool>(params.at("inverted")));
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
  ASSERT_TRUE(compile_and_run(cenv));
  ASSERT_NE(params.find("result"), params.end());
  // (5 + 3) * 4 - 10 = 8 * 4 - 10 = 32 - 10 = 22
  EXPECT_EQ(std::get<int64_t>(params.at("result")), 22);
}
