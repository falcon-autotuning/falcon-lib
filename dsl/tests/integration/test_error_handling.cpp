#include "dsl_test_base.hpp"
using namespace falcon::dsl;
using namespace falcon::dsl::test;
using namespace falcon::typing;

class ErrorHandlingTest : public DSLTestBase {};

TEST_F(ErrorHandlingTest, SimpleError) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/error_handling/simple-error.fal"),
      "SimpleError", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_GE(outputs.size(), 1);
  EXPECT_EQ(std::get<ErrorObject>(outputs[0]).message, "We are broken :<");
  EXPECT_FALSE(std::get<ErrorObject>(outputs[0]).is_fatal);
}
