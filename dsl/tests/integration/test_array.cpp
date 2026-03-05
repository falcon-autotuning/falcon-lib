#include "dsl_test_base.hpp"
#include <falcon-typing/PrimitiveTypes.hpp>

using namespace falcon::dsl;
using namespace falcon::dsl::test;
using namespace falcon::typing;

class ArrayMethodTest : public DSLTestBase {};

// ---------------------------------------------------------------------------
// Helper: build a shared_ptr<ArrayValue> containing int64_t elements
// ---------------------------------------------------------------------------
static std::shared_ptr<ArrayValue> make_int_array(std::vector<int64_t> vals) {
  auto arr = std::make_shared<ArrayValue>("int");
  for (auto v : vals) {
    arr->elements.push_back(RuntimeValue{v});
  }
  return arr;
}

// ---------------------------------------------------------------------------
// erase
// ---------------------------------------------------------------------------

// Erase the middle element [10, 20, 30] -> erase(1) -> [10, 30]
TEST_F(ArrayMethodTest, EraseMiddleElement) {
  ParameterMap params;
  params["arr"] = make_int_array({10, 20, 30});
  params["erase_idx"] = static_cast<int64_t>(1);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/erase-test.fal"),
      "EraseTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 2u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 10); // first
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 30); // second
}

// Erase the first element [10, 20, 30] -> erase(0) -> [20, 30]
TEST_F(ArrayMethodTest, EraseFirstElement) {
  ParameterMap params;
  params["arr"] = make_int_array({10, 20, 30});
  params["erase_idx"] = static_cast<int64_t>(0);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/erase-test.fal"),
      "EraseTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 2u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 20);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 30);
}

// Erase the last element [10, 20, 30] -> erase(2) -> [10, 20]
TEST_F(ArrayMethodTest, EraseLastElement) {
  ParameterMap params;
  params["arr"] = make_int_array({10, 20, 30});
  params["erase_idx"] = static_cast<int64_t>(2);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/erase-test.fal"),
      "EraseTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 2u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 10);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 20);
}

// ---------------------------------------------------------------------------
// insert
// ---------------------------------------------------------------------------

// Insert at front: [20, 30] -> insert(0, 10) -> [10, 20, 30]
TEST_F(ArrayMethodTest, InsertAtFront) {
  ParameterMap params;
  params["arr"] = make_int_array({20, 30});
  params["insert_idx"] = static_cast<int64_t>(0);
  params["insert_val"] = static_cast<int64_t>(10);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/insert-test.fal"),
      "InsertTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 3u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 10);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 20);
  EXPECT_EQ(std::get<int64_t>(outputs[2]), 30);
}

// Insert in the middle: [10, 30] -> insert(1, 20) -> [10, 20, 30]
TEST_F(ArrayMethodTest, InsertInMiddle) {
  ParameterMap params;
  params["arr"] = make_int_array({10, 30});
  params["insert_idx"] = static_cast<int64_t>(1);
  params["insert_val"] = static_cast<int64_t>(20);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/insert-test.fal"),
      "InsertTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 3u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 10);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 20);
  EXPECT_EQ(std::get<int64_t>(outputs[2]), 30);
}

// Insert at end: [10, 20] -> insert(2, 30) -> [10, 20, 30]
TEST_F(ArrayMethodTest, InsertAtEnd) {
  ParameterMap params;
  params["arr"] = make_int_array({10, 20});
  params["insert_idx"] = static_cast<int64_t>(2);
  params["insert_val"] = static_cast<int64_t>(30);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/insert-test.fal"),
      "InsertTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 3u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 10);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 20);
  EXPECT_EQ(std::get<int64_t>(outputs[2]), 30);
}

// ---------------------------------------------------------------------------
// pushback
// ---------------------------------------------------------------------------

// Push onto a 2-element array: [10, 20] -> pushback(30) -> arr[2] == 30
TEST_F(ArrayMethodTest, PushbackAppendsToEnd) {
  ParameterMap params;
  params["arr"] = make_int_array({10, 20});
  params["push_val"] = static_cast<int64_t>(30);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/pushback-test.fal"),
      "PushbackTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 1u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 30);
}

// Push onto an empty array: [] -> pushback(42) -> arr[0] == 42
// (uses index 0 since the array starts empty; the script reads arr[2],
//  so we seed it with two elements and push a distinct sentinel value)
TEST_F(ArrayMethodTest, PushbackOnTwoElementArray) {
  ParameterMap params;
  params["arr"] = make_int_array({1, 2});
  params["push_val"] = static_cast<int64_t>(99);

  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/pushback-test.fal"),
      "PushbackTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 1u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 99);
}

TEST_F(ArrayMethodTest, ArrayAssignment) {
  ParameterMap params;
  SingleCompileEnvironment cenv{
      std::filesystem::path("test-autotuners/array/assignment.fal"),
      "AssignmentTest", params, true};
  auto [success, outputs] = compile_and_run(cenv);
  ASSERT_TRUE(success);
  ASSERT_EQ(outputs.size(), 3u);
  EXPECT_EQ(std::get<int64_t>(outputs[0]), 1);
  EXPECT_EQ(std::get<int64_t>(outputs[1]), 2);
  EXPECT_EQ(std::get<int64_t>(outputs[2]), 3);
}
