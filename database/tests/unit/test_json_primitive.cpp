#include "falcon-database/DeviceCharacteristic.hpp"
#include <gtest/gtest.h>

using namespace falcon::database;

TEST(JSONPrimitiveTest, NullValue) {
  JSONPrimitive jsonp;
  EXPECT_TRUE(jsonp.is_null());
  EXPECT_EQ(jsonp.type(), JSONPrimitive::Type::Null);

  auto json = jsonp.to_json();
  EXPECT_TRUE(json.is_null());
}

TEST(JSONPrimitiveTest, BooleanValue) {
  JSONPrimitive jsonp(true);
  EXPECT_FALSE(jsonp.is_null());
  EXPECT_EQ(jsonp.type(), JSONPrimitive::Type::Boolean);
  EXPECT_TRUE(jsonp.as_bool());

  auto json = jsonp.to_json();
  EXPECT_TRUE(json.is_boolean());
  EXPECT_TRUE(json.get<bool>());
}

TEST(JSONPrimitiveTest, IntegerValue) {
  JSONPrimitive jsonp(int64_t(42)); // Explicitly use int64_t
  EXPECT_EQ(jsonp.type(), JSONPrimitive::Type::Integer);
  EXPECT_EQ(jsonp.as_int(), 42);

  auto json = jsonp.to_json();
  EXPECT_TRUE(json.is_number_integer());
  EXPECT_EQ(json.get<int64_t>(), 42);
}

TEST(JSONPrimitiveTest, DoubleValue) {
  JSONPrimitive jsonp(3.14159);
  EXPECT_EQ(jsonp.type(), JSONPrimitive::Type::Double);
  EXPECT_DOUBLE_EQ(jsonp.as_double(), 3.14159);

  auto json = jsonp.to_json();
  EXPECT_TRUE(json.is_number_float());
  EXPECT_DOUBLE_EQ(json.get<double>(), 3.14159);
}

TEST(JSONPrimitiveTest, StringValue) {
  JSONPrimitive jsonp("test_string");
  EXPECT_EQ(jsonp.type(), JSONPrimitive::Type::String);
  EXPECT_EQ(jsonp.as_string(), "test_string");

  auto json = jsonp.to_json();
  EXPECT_TRUE(json.is_string());
  EXPECT_EQ(json.get<std::string>(), "test_string");
}

TEST(JSONPrimitiveTest, FromJson) {
  json j_bool = true;
  auto jsonp_bool = JSONPrimitive::from_json(j_bool);
  EXPECT_EQ(jsonp_bool.type(), JSONPrimitive::Type::Boolean);
  EXPECT_TRUE(jsonp_bool.as_bool());

  json j_int = 123;
  auto jsonp_int = JSONPrimitive::from_json(j_int);
  EXPECT_EQ(jsonp_int.type(), JSONPrimitive::Type::Integer);
  EXPECT_EQ(jsonp_int.as_int(), 123);

  json j_double = 2.718;
  auto jsonp_double = JSONPrimitive::from_json(j_double);
  EXPECT_EQ(jsonp_double.type(), JSONPrimitive::Type::Double);
  EXPECT_DOUBLE_EQ(jsonp_double.as_double(), 2.718);

  json j_string = "hello";
  auto jsonp_string = JSONPrimitive::from_json(j_string);
  EXPECT_EQ(jsonp_string.type(), JSONPrimitive::Type::String);
  EXPECT_EQ(jsonp_string.as_string(), "hello");
}

TEST(JSONPrimitiveTest, TypeMismatchThrows) {
  JSONPrimitive jsonp(int64_t(42)); // Explicitly int64_t
  EXPECT_THROW((void)jsonp.as_bool(), std::runtime_error);
  EXPECT_THROW((void)jsonp.as_double(), std::runtime_error);
  EXPECT_THROW((void)jsonp.as_string(), std::runtime_error);
}
