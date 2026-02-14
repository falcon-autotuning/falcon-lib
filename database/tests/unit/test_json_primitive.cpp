#include "falcon-database/DeviceCharacteristic.hpp"
#include <gtest/gtest.h>

using namespace falcon::database;

TEST(JSONPrimitiveTest, NullValue) {
  JSONPrimitive jp;
  EXPECT_TRUE(jp.is_null());
  EXPECT_EQ(jp.type(), JSONPrimitive::Type::Null);

  auto j = jp.to_json();
  EXPECT_TRUE(j.is_null());
}

TEST(JSONPrimitiveTest, BooleanValue) {
  JSONPrimitive jp(true);
  EXPECT_FALSE(jp.is_null());
  EXPECT_EQ(jp.type(), JSONPrimitive::Type::Boolean);
  EXPECT_TRUE(jp.as_bool());

  auto j = jp.to_json();
  EXPECT_TRUE(j.is_boolean());
  EXPECT_TRUE(j.get<bool>());
}

TEST(JSONPrimitiveTest, IntegerValue) {
  JSONPrimitive jp(int64_t(42)); // Explicitly use int64_t
  EXPECT_EQ(jp.type(), JSONPrimitive::Type::Integer);
  EXPECT_EQ(jp.as_int(), 42);

  auto j = jp.to_json();
  EXPECT_TRUE(j.is_number_integer());
  EXPECT_EQ(j.get<int64_t>(), 42);
}

TEST(JSONPrimitiveTest, DoubleValue) {
  JSONPrimitive jp(3.14159);
  EXPECT_EQ(jp.type(), JSONPrimitive::Type::Double);
  EXPECT_DOUBLE_EQ(jp.as_double(), 3.14159);

  auto j = jp.to_json();
  EXPECT_TRUE(j.is_number_float());
  EXPECT_DOUBLE_EQ(j.get<double>(), 3.14159);
}

TEST(JSONPrimitiveTest, StringValue) {
  JSONPrimitive jp("test_string");
  EXPECT_EQ(jp.type(), JSONPrimitive::Type::String);
  EXPECT_EQ(jp.as_string(), "test_string");

  auto j = jp.to_json();
  EXPECT_TRUE(j.is_string());
  EXPECT_EQ(j.get<std::string>(), "test_string");
}

TEST(JSONPrimitiveTest, FromJson) {
  json j_bool = true;
  auto jp_bool = JSONPrimitive::from_json(j_bool);
  EXPECT_EQ(jp_bool.type(), JSONPrimitive::Type::Boolean);
  EXPECT_TRUE(jp_bool.as_bool());

  json j_int = 123;
  auto jp_int = JSONPrimitive::from_json(j_int);
  EXPECT_EQ(jp_int.type(), JSONPrimitive::Type::Integer);
  EXPECT_EQ(jp_int.as_int(), 123);

  json j_double = 2.718;
  auto jp_double = JSONPrimitive::from_json(j_double);
  EXPECT_EQ(jp_double.type(), JSONPrimitive::Type::Double);
  EXPECT_DOUBLE_EQ(jp_double.as_double(), 2.718);

  json j_string = "hello";
  auto jp_string = JSONPrimitive::from_json(j_string);
  EXPECT_EQ(jp_string.type(), JSONPrimitive::Type::String);
  EXPECT_EQ(jp_string.as_string(), "hello");
}

TEST(JSONPrimitiveTest, TypeMismatchThrows) {
  JSONPrimitive jp(int64_t(42)); // Explicitly int64_t
  EXPECT_THROW((void)jp.as_bool(), std::runtime_error);
  EXPECT_THROW((void)jp.as_double(), std::runtime_error);
  EXPECT_THROW((void)jp.as_string(), std::runtime_error);
}
