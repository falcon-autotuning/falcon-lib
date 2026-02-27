#include "dsl_test_base.hpp"
#include <iostream>
#include <sstream>
#include <streambuf>
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

class LogTest : public DSLTestBase {};

TEST_F(LogTest, SimpleNest) {
  std::stringstream captured;
  std::streambuf *old_buf = std::cout.rdbuf(captured.rdbuf());

  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/log/log.fal"), "LogTest", params,
      true};
  auto [success, outputs] = compile_and_run(cenv);

  std::cout.rdbuf(old_buf); // Restore

  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 0);

  std::string output = captured.str();
  ASSERT_NE(output.find("This is an info message"), std::string::npos);
  ASSERT_NE(output.find("This is a warning message"), std::string::npos);
  ASSERT_NE(output.find("This is an error message"), std::string::npos);
}
