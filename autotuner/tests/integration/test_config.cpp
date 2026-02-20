#include "dsl_test_base.hpp"
#include "falcon_core/autotuner_interfaces/names/Gname.hpp"
using namespace falcon::autotuner;
using namespace falcon::autotuner::test;
class ConfigTest : public DSLTestBase {};
// TEST_F(ConfigTest, SequentialStates) {
//   ParameterMap params;
//   falcon_core::autotuner_interfaces::names::GnameSP gname =
//       std::make_shared<falcon_core::autotuner_interfaces::names::Gname>(1);
//   params.set("gname", gname, atc::ParamType::Gname);
//   ASSERT_TRUE(compile_and_run(
//       std::filesystem::path("test-autotuners/generic-iteration-test.fal"),
//       "GenericIterationTest", params));
//
//   EXPECT_EQ(params.get<int64_t>("counter"), 2);
// }
