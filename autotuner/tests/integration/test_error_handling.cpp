// autotuner/tests/dsl/test_error_handling.cpp
#include "dsl_test_base.hpp"

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class ErrorHandlingTest : public DSLTestBase {};

TEST_F(ErrorHandlingTest, ErrorStateReached) {
  const char *dsl = R"(
autotuner ErrorTest (bool trigger_error) [] -> (string error) {
  params {
    string error = "";
  }
  
  start -> check;
  
  state check {
    if (trigger_error == true) -> error_state;
    else -> success;
  }
  
  state success {
    terminal;
  }
  
  state error_state {
    error = "triggered";
    terminal;
  }
}
)";

  ParameterMap params;
  params.set("trigger_error", true);

  ASSERT_TRUE(compile_and_run(dsl, "ErrorTest", params));

  EXPECT_EQ(params.get<std::string>("error"), "triggered");
}

TEST_F(ErrorHandlingTest, ValidationErrors) {
  const char *dsl = R"(
autotuner ValidationTest (int value) [] -> (string error) {
  params {
    string error = "none";
  }
  
  start -> validate;
  
  state validate {
    if (value < 0) -> negative_error;
    else if (value > 100) -> overflow_error;
    else -> success;
  }
  
  state negative_error {
    error = "negative_value";
    terminal;
  }
  
  state overflow_error {
    error = "overflow";
    terminal;
  }
  
  state success {
    terminal;
  }
}
)";

  // Test negative error
  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(-10));
    ASSERT_TRUE(compile_and_run(dsl, "ValidationTest", params));
    EXPECT_EQ(params.get<std::string>("error"), "negative_value");
  }

  // Test overflow error
  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(150));
    ASSERT_TRUE(compile_and_run(dsl, "ValidationTest", params));
    EXPECT_EQ(params.get<std::string>("error"), "overflow");
  }

  // Test success
  {
    ParameterMap params;
    params.set("value", static_cast<int64_t>(50));
    ASSERT_TRUE(compile_and_run(dsl, "ValidationTest", params));
    EXPECT_EQ(params.get<std::string>("error"), "none");
  }
}

TEST_F(ErrorHandlingTest, FailKeyword) {
  const char *dsl = R"(
autotuner FailTest (bool ready) [] -> () {
  params {}
  
  start -> check_ready;
  
  state check_ready {
    if (ready) success;
    else fail "Device not ready";
  }
}
)";

  // Test fail case
  {
    ParameterMap params;
    params.set("ready", false);

    // This should fail
    ASSERT_FALSE(compile_and_run(dsl, "FailTest", params, false));
  }

  // Test success case
  {
    ParameterMap params;
    params.set("ready", true);

    ASSERT_TRUE(compile_and_run(dsl, "FailTest", params, true));
  }
}
