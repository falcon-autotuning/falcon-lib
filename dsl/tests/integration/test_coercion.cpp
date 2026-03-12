#include "dsl_test_base.hpp"
using namespace falcon::dsl;
using namespace falcon::dsl::test;
using namespace falcon::typing;

class CoercionTest : public DSLTestBase {};

TEST_F(CoercionTest, StringToInt) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/expressions/coercion-test.fal"),
      "CoercionTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 4);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 42);
  EXPECT_NEAR(std::get<double>(outputs[1]), 3.14, 0.001);
  EXPECT_TRUE(std::get<bool>(outputs[2]));
  EXPECT_EQ(std::get<std::string>(outputs[3]), "123");
}
