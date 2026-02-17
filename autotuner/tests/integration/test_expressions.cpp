// autotuner/tests/dsl/test_expressions.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class ExpressionsTest : public DSLTestBase {};

TEST_F(ExpressionsTest, ArithmeticOperations) {
  const char *dsl = R"(
autotuner ArithmeticTest (float x, float y) [] -> (float add, float sub, float mul, float div) {
  params {
    float add = 0.0;
    float sub = 0.0;
    float mul = 0.0;
    float div = 0.0;
  }
  
  start -> calculate;
  
  state calculate {
    add = x + y;
    sub = x - y;
    mul = x * y;
    div = x / y;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("x", 10.0);
  params.set("y", 3.0);

  ASSERT_TRUE(compile_and_run(dsl, "ArithmeticTest", params));

  EXPECT_DOUBLE_EQ(params.get<double>("add"), 13.0);
  EXPECT_DOUBLE_EQ(params.get<double>("sub"), 7.0);
  EXPECT_DOUBLE_EQ(params.get<double>("mul"), 30.0);
  EXPECT_NEAR(params.get<double>("div"), 3.333, 0.01);
}

TEST_F(ExpressionsTest, ComparisonOperators) {
  const char *dsl = R"(
autotuner ComparisonTest (int a, int b) [] -> (bool gt, bool lt, bool eq, bool gte, bool lte) {
  params {
    bool gt = false;
    bool lt = false;
    bool eq = false;
    bool gte = false;
    bool lte = false;
  }
  
  start -> compare;
  
  state compare {
    gt = a > b;
    lt = a < b;
    eq = a == b;
    gte = a >= b;
    lte = a <= b;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("a", static_cast<int64_t>(10));
  params.set("b", static_cast<int64_t>(20));

  ASSERT_TRUE(compile_and_run(dsl, "ComparisonTest", params));

  EXPECT_FALSE(params.get<bool>("gt"));
  EXPECT_TRUE(params.get<bool>("lt"));
  EXPECT_FALSE(params.get<bool>("eq"));
  EXPECT_FALSE(params.get<bool>("gte"));
  EXPECT_TRUE(params.get<bool>("lte"));
}

TEST_F(ExpressionsTest, LogicalOperations) {
  const char *dsl = R"(
autotuner LogicalTest (bool flag1, bool flag2) [] -> (string result) {
  params {
    string result = "";
  }
  
  start -> evaluate;
  
  state evaluate {
    if (flag1 == true) {
      if (flag2 == false) {
        result = "expected";
        -> done;
      }
      else -> error_state;
    }
    else -> error_state;
  }
  
  state done {
    terminal;
  }
  
  state error_state {
    result = "unexpected";
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("flag1", true);
  params.set("flag2", false);

  ASSERT_TRUE(compile_and_run(dsl, "LogicalTest", params));

  EXPECT_EQ(params.get<std::string>("result"), "expected");
}

TEST_F(ExpressionsTest, UnaryOperators) {
  const char *dsl = R"(
autotuner UnaryTest (int value, bool flag) [] -> (int negated, bool inverted) {
  params {
    int negated = 0;
    bool inverted = false;
  }
  
  start -> compute;
  
  state compute {
    negated = -value;
    inverted = !flag;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("value", static_cast<int64_t>(42));
  params.set("flag", true);

  ASSERT_TRUE(compile_and_run(dsl, "UnaryTest", params));

  EXPECT_EQ(params.get<int64_t>("negated"), -42);
  EXPECT_FALSE(params.get<bool>("inverted"));
}

TEST_F(ExpressionsTest, ComplexExpressions) {
  const char *dsl = R"(
autotuner ComplexExprTest (int a, int b, int c) [] -> (int result) {
  params {
    int result = 0;
  }
  
  start -> compute;
  
  state compute {
    result = (a + b) * c - 10;
    -> done;
  }
  
  state done {
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("a", static_cast<int64_t>(5));
  params.set("b", static_cast<int64_t>(3));
  params.set("c", static_cast<int64_t>(4));

  ASSERT_TRUE(compile_and_run(dsl, "ComplexExprTest", params));

  // (5 + 3) * 4 - 10 = 8 * 4 - 10 = 32 - 10 = 22
  EXPECT_EQ(params.get<int64_t>("result"), 22);
}
