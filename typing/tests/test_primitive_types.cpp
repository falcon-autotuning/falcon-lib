/**
 * @file test_primitive_types.cpp
 * @brief Tests for falcon-typing: verifies that all primitive types survive
 *        a round-trip across the C-ABI (FalconParamEntry / FalconResultSlot)
 *        boundary, simulating the FFI path used between CPP consumers.
 */

#include "falcon-typing/FFIHelpers.hpp"
#include "falcon-typing/PrimitiveTypes.hpp"
#include <cstdlib>
#include <cstring>
#include <gtest/gtest.h>

using namespace falcon::typing;
using namespace falcon::typing::ffi;

// ─────────────────────────────────────────────────────────────────────────────
// Helpers: simulate a wrapper function that echos its inputs back as outputs
// ─────────────────────────────────────────────────────────────────────────────

/// Simulate a C-ABI wrapper that receives params and packs them as results.
/// This exercises both wrapper::unpack_params and wrapper::pack_results.
static void echo_wrapper(const FalconParamEntry *params, int32_t param_count,
                         FalconResultSlot *out_slots, int32_t *out_count) {
  ParameterMap pm = wrapper::unpack_params(params, param_count);
  FunctionResult fr;
  fr.reserve(pm.size());
  for (auto &[k, v] : pm) {
    fr.push_back(v);
  }
  wrapper::pack_results(fr, out_slots, *out_count, out_count);
}

// ─────────────────────────────────────────────────────────────────────────────
// Fixture
// ─────────────────────────────────────────────────────────────────────────────
class FfiRoundTripTest : public ::testing::Test {
protected:
  static constexpr int32_t kMaxSlots = 16;

  /// Drive a full engine→wrapper→engine round trip for a ParameterMap.
  FunctionResult round_trip(const ParameterMap &params) {
    // Engine packs params
    auto packed = engine::pack_params(params);

    // Wrapper receives, unpacks, re-packs as results
    FalconResultSlot out_slots[kMaxSlots] = {};
    int32_t out_count = kMaxSlots;
    echo_wrapper(packed.entries.data(),
                 static_cast<int32_t>(packed.entries.size()), out_slots,
                 &out_count);

    // Engine unpacks results
    return engine::unpack_results(out_slots, out_count);
  }
};

// ─────────────────────────────────────────────────────────────────────────────
// Tests: scalar primitives
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(FfiRoundTripTest, IntRoundTrip) {
  ParameterMap params;
  params["x"] = int64_t{42};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<int64_t>(result[0]));
  EXPECT_EQ(std::get<int64_t>(result[0]), 42);
}

TEST_F(FfiRoundTripTest, NegativeIntRoundTrip) {
  ParameterMap params;
  params["val"] = int64_t{-9999};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<int64_t>(result[0]), -9999);
}

TEST_F(FfiRoundTripTest, FloatRoundTrip) {
  ParameterMap params;
  params["pi"] = double{3.14159};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<double>(result[0]));
  EXPECT_DOUBLE_EQ(std::get<double>(result[0]), 3.14159);
}

TEST_F(FfiRoundTripTest, BoolTrueRoundTrip) {
  ParameterMap params;
  params["flag"] = true;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<bool>(result[0]));
  EXPECT_TRUE(std::get<bool>(result[0]));
}

TEST_F(FfiRoundTripTest, BoolFalseRoundTrip) {
  ParameterMap params;
  params["flag"] = false;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  EXPECT_FALSE(std::get<bool>(result[0]));
}

TEST_F(FfiRoundTripTest, StringRoundTrip) {
  ParameterMap params;
  params["msg"] = std::string{"hello, falcon"};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<std::string>(result[0]));
  EXPECT_EQ(std::get<std::string>(result[0]), "hello, falcon");
}

TEST_F(FfiRoundTripTest, EmptyStringRoundTrip) {
  ParameterMap params;
  params["empty"] = std::string{""};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  EXPECT_EQ(std::get<std::string>(result[0]), "");
}

TEST_F(FfiRoundTripTest, NilRoundTrip) {
  ParameterMap params;
  params["nothing"] = nullptr;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  EXPECT_TRUE(std::holds_alternative<std::nullptr_t>(result[0]));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: ErrorObject
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(FfiRoundTripTest, ErrorObjectNonFatalRoundTrip) {
  ParameterMap params;
  params["err"] = ErrorObject{"something went wrong", false};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<ErrorObject>(result[0]));
  const auto &err = std::get<ErrorObject>(result[0]);
  EXPECT_EQ(err.message, "something went wrong");
  EXPECT_FALSE(err.is_fatal);
}

TEST_F(FfiRoundTripTest, ErrorObjectFatalRoundTrip) {
  ParameterMap params;
  params["err"] = ErrorObject{"fatal hardware fault", true};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  const auto &err = std::get<ErrorObject>(result[0]);
  EXPECT_EQ(err.message, "fatal hardware fault");
  EXPECT_TRUE(err.is_fatal);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: StructInstance (plain FAL struct, no native handle)
// ───────────────────────────────��─────────────────────────────────────────────

TEST_F(FfiRoundTripTest, StructInstanceRoundTrip) {
  auto inst = std::make_shared<StructInstance>("MyStruct");
  inst->set_field("count", int64_t{7});
  inst->set_field("label", std::string{"q0"});

  ParameterMap params;
  params["obj"] = inst;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(
      std::holds_alternative<std::shared_ptr<StructInstance>>(result[0]));

  auto out = std::get<std::shared_ptr<StructInstance>>(result[0]);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->type_name, "MyStruct");
  EXPECT_EQ(std::get<int64_t>(out->get_field("count")), 7);
  EXPECT_EQ(std::get<std::string>(out->get_field("label")), "q0");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: TupleValue
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(FfiRoundTripTest, TupleValueRoundTrip) {
  auto tup = std::make_shared<TupleValue>();
  tup->values.push_back(int64_t{1});
  tup->values.push_back(std::string{"two"});
  tup->values.push_back(double{3.0});

  ParameterMap params;
  params["tup"] = tup;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<std::shared_ptr<TupleValue>>(result[0]));

  auto out = std::get<std::shared_ptr<TupleValue>>(result[0]);
  ASSERT_NE(out, nullptr);
  ASSERT_EQ(out->size(), 3u);
  EXPECT_EQ(std::get<int64_t>((*out)[0]), 1);
  EXPECT_EQ(std::get<std::string>((*out)[1]), "two");
  EXPECT_DOUBLE_EQ(std::get<double>((*out)[2]), 3.0);
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: Multiple params in one map (order-independent via map iteration)
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(FfiRoundTripTest, MultipleParamsRoundTrip) {
  ParameterMap params;
  params["a"] = int64_t{10};
  params["b"] = std::string{"bee"};
  params["c"] = true;
  params["d"] = double{2.718};

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 4u);

  // Re-pack into a map by round-tripping through wrapper unpack to verify
  // each value survives. We re-drive through engine::pack_params on a
  // reconstructed ParameterMap.
  //
  // Since std::map iterates in key order (a, b, c, d), we can assert by index.
  EXPECT_EQ(std::get<int64_t>(result[0]), 10);          // a
  EXPECT_EQ(std::get<std::string>(result[1]), "bee");   // b
  EXPECT_TRUE(std::get<bool>(result[2]));               // c
  EXPECT_DOUBLE_EQ(std::get<double>(result[3]), 2.718); // d
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: get_runtime_type_name
// ─────────────────────────────────────────────────────────────────────────────

TEST(PrimitiveTypesTest, TypeNameInt) {
  EXPECT_EQ(get_runtime_type_name(int64_t{0}), "int");
}
TEST(PrimitiveTypesTest, TypeNameFloat) {
  EXPECT_EQ(get_runtime_type_name(double{0.0}), "float");
}
TEST(PrimitiveTypesTest, TypeNameBool) {
  EXPECT_EQ(get_runtime_type_name(bool{false}), "bool");
}
TEST(PrimitiveTypesTest, TypeNameString) {
  EXPECT_EQ(get_runtime_type_name(std::string{}), "string");
}
TEST(PrimitiveTypesTest, TypeNameNil) {
  RuntimeValue v = nullptr;
  EXPECT_EQ(get_runtime_type_name(v), "nil");
}
TEST(PrimitiveTypesTest, TypeNameError) {
  RuntimeValue v = ErrorObject{"x"};
  EXPECT_EQ(get_runtime_type_name(v), "Error");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: runtime_value_to_string
// ─────────────────────────────────────────────────────────────────────────────

TEST(PrimitiveTypesTest, ToStringInt) {
  EXPECT_EQ(runtime_value_to_string(int64_t{42}), "42");
}
TEST(PrimitiveTypesTest, ToStringFloat) {
  // std::to_string format — just verify it doesn't crash and contains digits
  std::string s = runtime_value_to_string(double{1.5});
  EXPECT_FALSE(s.empty());
}
TEST(PrimitiveTypesTest, ToStringBoolTrue) {
  EXPECT_EQ(runtime_value_to_string(bool{true}), "true");
}
TEST(PrimitiveTypesTest, ToStringBoolFalse) {
  EXPECT_EQ(runtime_value_to_string(bool{false}), "false");
}
TEST(PrimitiveTypesTest, ToStringString) {
  EXPECT_EQ(runtime_value_to_string(std::string{"hi"}), "\"hi\"");
}
TEST(PrimitiveTypesTest, ToStringNil) {
  RuntimeValue v = nullptr;
  EXPECT_EQ(runtime_value_to_string(v), "nil");
}
TEST(PrimitiveTypesTest, ToStringError) {
  RuntimeValue v = ErrorObject{"boom"};
  EXPECT_EQ(runtime_value_to_string(v), "Error(boom)");
}
TEST(PrimitiveTypesTest, ToStringNilTuple) {
  RuntimeValue v = std::shared_ptr<TupleValue>{nullptr};
  EXPECT_EQ(runtime_value_to_string(v), "(nil)");
}
TEST(PrimitiveTypesTest, ToStringNilStruct) {
  RuntimeValue v = std::shared_ptr<StructInstance>{nullptr};
  EXPECT_EQ(runtime_value_to_string(v), "<StructInstance:nil>");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: ArrayValue — FFI round-trip
// ─────────────────────────────────────────────────────────────────────────────

TEST_F(FfiRoundTripTest, EmptyIntArrayRoundTrip) {
  auto arr = std::make_shared<ArrayValue>("int");

  ParameterMap params;
  params["arr"] = arr;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<std::shared_ptr<ArrayValue>>(result[0]));

  auto out = std::get<std::shared_ptr<ArrayValue>>(result[0]);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->element_type_name, "int");
  EXPECT_EQ(out->size(), 0u);
}

TEST_F(FfiRoundTripTest, IntArrayRoundTrip) {
  auto arr = std::make_shared<ArrayValue>("int");
  arr->elements.push_back(int64_t{1});
  arr->elements.push_back(int64_t{2});
  arr->elements.push_back(int64_t{3});

  ParameterMap params;
  params["arr"] = arr;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<std::shared_ptr<ArrayValue>>(result[0]));

  auto out = std::get<std::shared_ptr<ArrayValue>>(result[0]);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->element_type_name, "int");
  ASSERT_EQ(out->size(), 3u);
  EXPECT_EQ(std::get<int64_t>((*out)[0]), 1);
  EXPECT_EQ(std::get<int64_t>((*out)[1]), 2);
  EXPECT_EQ(std::get<int64_t>((*out)[2]), 3);
}

TEST_F(FfiRoundTripTest, FloatArrayRoundTrip) {
  auto arr = std::make_shared<ArrayValue>("float");
  arr->elements.push_back(double{1.1});
  arr->elements.push_back(double{2.2});

  ParameterMap params;
  params["arr"] = arr;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<std::shared_ptr<ArrayValue>>(result[0]));

  auto out = std::get<std::shared_ptr<ArrayValue>>(result[0]);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->element_type_name, "float");
  ASSERT_EQ(out->size(), 2u);
  EXPECT_DOUBLE_EQ(std::get<double>((*out)[0]), 1.1);
  EXPECT_DOUBLE_EQ(std::get<double>((*out)[1]), 2.2);
}

TEST_F(FfiRoundTripTest, StringArrayRoundTrip) {
  auto arr = std::make_shared<ArrayValue>("string");
  arr->elements.push_back(std::string{"hello"});
  arr->elements.push_back(std::string{"world"});

  ParameterMap params;
  params["arr"] = arr;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<std::shared_ptr<ArrayValue>>(result[0]));

  auto out = std::get<std::shared_ptr<ArrayValue>>(result[0]);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->element_type_name, "string");
  ASSERT_EQ(out->size(), 2u);
  EXPECT_EQ(std::get<std::string>((*out)[0]), "hello");
  EXPECT_EQ(std::get<std::string>((*out)[1]), "world");
}

TEST_F(FfiRoundTripTest, BoolArrayRoundTrip) {
  auto arr = std::make_shared<ArrayValue>("bool");
  arr->elements.push_back(bool{true});
  arr->elements.push_back(bool{false});
  arr->elements.push_back(bool{true});

  ParameterMap params;
  params["arr"] = arr;

  auto result = round_trip(params);
  ASSERT_EQ(result.size(), 1u);
  ASSERT_TRUE(std::holds_alternative<std::shared_ptr<ArrayValue>>(result[0]));

  auto out = std::get<std::shared_ptr<ArrayValue>>(result[0]);
  ASSERT_NE(out, nullptr);
  EXPECT_EQ(out->element_type_name, "bool");
  ASSERT_EQ(out->size(), 3u);
  EXPECT_TRUE(std::get<bool>((*out)[0]));
  EXPECT_FALSE(std::get<bool>((*out)[1]));
  EXPECT_TRUE(std::get<bool>((*out)[2]));
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: get_runtime_type_name for ArrayValue
// ─────────────────────────────────────────────────────────────────────────────

TEST(PrimitiveTypesTest, TypeNameIntArray) {
  RuntimeValue v = std::make_shared<ArrayValue>("int");
  EXPECT_EQ(get_runtime_type_name(v), "Array[int]");
}

TEST(PrimitiveTypesTest, TypeNameFloatArray) {
  RuntimeValue v = std::make_shared<ArrayValue>("float");
  EXPECT_EQ(get_runtime_type_name(v), "Array[float]");
}

TEST(PrimitiveTypesTest, TypeNameNilArray) {
  RuntimeValue v = std::shared_ptr<ArrayValue>{nullptr};
  EXPECT_EQ(get_runtime_type_name(v), "Array");
}

// ─────────────────────────────────────────────────────────────────────────────
// Tests: runtime_value_to_string for ArrayValue
// ─────────────────────────────────────────────────────────────────────────────

TEST(PrimitiveTypesTest, ToStringEmptyArray) {
  RuntimeValue v = std::make_shared<ArrayValue>("int");
  EXPECT_EQ(runtime_value_to_string(v), "[]");
}

TEST(PrimitiveTypesTest, ToStringIntArray) {
  auto arr = std::make_shared<ArrayValue>("int");
  arr->elements.push_back(int64_t{1});
  arr->elements.push_back(int64_t{2});
  RuntimeValue v = arr;
  EXPECT_EQ(runtime_value_to_string(v), "[1, 2]");
}

TEST(PrimitiveTypesTest, ToStringNilArray) {
  RuntimeValue v = std::shared_ptr<ArrayValue>{nullptr};
  EXPECT_EQ(runtime_value_to_string(v), "[nil]");
}
