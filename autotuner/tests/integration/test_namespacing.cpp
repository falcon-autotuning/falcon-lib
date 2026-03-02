#include "dsl_test_base.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class NamespacingTest : public DSLTestBase {};

TEST_F(NamespacingTest, ScopedAndTransitiveCalls) {
  ParameterMap params;
  params["a"] = static_cast<int64_t>(10);
  params["b"] = static_cast<int64_t>(20);

  // Load the main file which imports geometry.fal, which imports math_utils.fal
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/namespacing/main.fal"),
      "NamespacingTest", params, true};

  auto [success, outputs] = compile_and_run(cenv);

  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 3);

  // sum = geometry::call_math_add_scoped(10, 20) -> math_utils::add(10, 20) ->
  // 30
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 30);

  // area = geometry::area_square(5.0) -> 25.0
  EXPECT_NEAR(std::get<double>(outputs[1]), 25.0, 1e-9);

  // version = math_utils::get_version() -> "1.0.0"
  EXPECT_EQ(std::get<std::string>(outputs[2]), "1.0.0");
}

TEST_F(NamespacingTest, CollisionDisambiguation) {
  // Create two files with same routine name
  auto file1 = write_dsl_file(R"(
routine greet() -> (string msg) { msg = "Hello from A"; }
)");
  auto file2 = write_dsl_file(R"(
routine greet() -> (string msg) { msg = "Hello from B"; }
)");

  std::string mod1 = file1.stem().string();
  std::string mod2 = file2.stem().string();

  // Create an autotuner that calls both routines by their scoped names
  // We don't need routines call_a/call_b if we just run an autotuner that does
  // the calls
  auto main_file = write_dsl_file(
      "import \"" + file1.filename().string() + "\";\n" + "import \"" +
      file2.filename().string() + "\";\n\n" +
      "autotuner Collision () -> (string res_a, string res_b) {\n" +
      "  start -> run;\n" + "  state run {\n" + "    res_a = " + mod1 +
      "::greet();\n" + "    res_b = " + mod2 + "::greet();\n" +
      "    terminal;\n" + "  }\n" + "}\n");

  ParameterMap params;
  SingleCompileEnvironment cenv{main_file, "Collision", params, true};

  auto [success, outputs] = compile_and_run(cenv);

  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 2);
  EXPECT_EQ(std::get<std::string>(outputs[0]), "Hello from A");
  EXPECT_EQ(std::get<std::string>(outputs[1]), "Hello from B");
}

TEST_F(NamespacingTest, BareNameFallback) {
  auto lib = write_dsl_file(R"(
routine secret() -> (int val) { val = 42; }
)");
  auto main_file = write_dsl_file(
      "import \"" + lib.filename().string() + "\";\n\n" +
      "autotuner Fallback () -> (int val) {\n" + "  start -> run;\n" +
      "  state run {\n" + "    val = secret();\n" + "    terminal;\n" +
      "  }\n" + "}\n");

  ParameterMap params;
  SingleCompileEnvironment cenv{main_file, "Fallback", params, true};

  auto [success, outputs] = compile_and_run(cenv);

  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 1);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 42);
}
