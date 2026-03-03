#include "dsl_test_base.hpp"
using namespace falcon::dsl;
using namespace falcon::dsl::test;
using namespace falcon::typing;

class StructsTest : public DSLTestBase {};

TEST_F(StructsTest, QuantityStruct) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(5);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/structs/quantity-struct.fal"),
      "QuantityStruct", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 15);
}

TEST_F(StructsTest, MultipleStruct) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(5);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/structs/multiple-struct.fal"),
      "MultipleStruct", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 15);
  EXPECT_EQ(std::get<std::string>(outputs[1]), "test");
}

TEST_F(StructsTest, StructWithDefaults) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(5);
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/structs/struct-with-defaults.fal"),
      "QuantityStruct", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_FALSE(outputs.empty());
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 15);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 15);
}
