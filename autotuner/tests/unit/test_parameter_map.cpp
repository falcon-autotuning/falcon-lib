#include "falcon-autotuner/ParameterMap.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;

TEST(ParameterMapTest, SetAndGetInt) {
  ParameterMap params;
  params.set("counter", int64_t(42));

  EXPECT_TRUE(params.has("counter"));
  EXPECT_EQ(params.get<int64_t>("counter"), 42);
}

TEST(ParameterMapTest, SetAndGetFloat) {
  ParameterMap params;
  params.set("voltage", 1.5);

  EXPECT_TRUE(params.has("voltage"));
  EXPECT_DOUBLE_EQ(params.get<double>("voltage"), 1.5);
}

TEST(ParameterMapTest, SetAndGetBool) {
  ParameterMap params;
  params.set("success", true);

  EXPECT_TRUE(params.has("success"));
  EXPECT_TRUE(params.get<bool>("success"));
}

TEST(ParameterMapTest, SetAndGetString) {
  ParameterMap params;
  params.set("message", std::string("hello"));

  EXPECT_TRUE(params.has("message"));
  EXPECT_EQ(params.get<std::string>("message"), "hello");
}

TEST(ParameterMapTest, TryGetExisting) {
  ParameterMap params;
  params.set("value", int64_t(100));

  auto result = params.try_get<int64_t>("value");
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 100);
}

TEST(ParameterMapTest, TryGetNonExisting) {
  ParameterMap params;

  auto result = params.try_get<int64_t>("missing");
  EXPECT_FALSE(result.has_value());
}

TEST(ParameterMapTest, TryGetWrongType) {
  ParameterMap params;
  params.set("value", int64_t(100));

  auto result = params.try_get<double>("value");
  EXPECT_FALSE(result.has_value());
}

TEST(ParameterMapTest, Merge) {
  ParameterMap params1;
  params1.set("a", int64_t(1));
  params1.set("b", int64_t(2));

  ParameterMap params2;
  params2.set("b", int64_t(20));
  params2.set("c", int64_t(30));

  params1.merge(params2);

  EXPECT_EQ(params1.get<int64_t>("a"), 1);
  EXPECT_EQ(params1.get<int64_t>("b"), 20); // Overwritten
  EXPECT_EQ(params1.get<int64_t>("c"), 30);
}

TEST(ParameterMapTest, Keys) {
  ParameterMap params;
  params.set("a", int64_t(1));
  params.set("b", 2.0);
  params.set("c", true);

  auto keys = params.keys();
  EXPECT_EQ(keys.size(), 3);

  std::sort(keys.begin(), keys.end());
  EXPECT_EQ(keys[0], "a");
  EXPECT_EQ(keys[1], "b");
  EXPECT_EQ(keys[2], "c");
}

TEST(ParameterMapTest, Remove) {
  ParameterMap params;
  params.set("value", int64_t(42));

  EXPECT_TRUE(params.has("value"));

  params.remove("value");

  EXPECT_FALSE(params.has("value"));
}

TEST(ParameterMapTest, Clear) {
  ParameterMap params;
  params.set("a", int64_t(1));
  params.set("b", int64_t(2));

  EXPECT_EQ(params.size(), 2);

  params.clear();

  EXPECT_EQ(params.size(), 0);
}

TEST(ParameterMapTest, ToJson) {
  ParameterMap params;
  params.set("int_val", int64_t(42));
  params.set("float_val", 3.14);
  params.set("bool_val", true);
  params.set("string_val", std::string("test"));

  auto j = params.to_json();

  EXPECT_EQ(j["int_val"].get<int64_t>(), 42);
  EXPECT_DOUBLE_EQ(j["float_val"].get<double>(), 3.14);
  EXPECT_EQ(j["bool_val"].get<bool>(), true);
  EXPECT_EQ(j["string_val"].get<std::string>(), "test");
}

TEST(ParameterMapTest, FromJson) {
  nlohmann::json j = {{"int_val", 42},
                      {"float_val", 3.14},
                      {"bool_val", true},
                      {"string_val", "test"}};

  auto params = ParameterMap::from_json(j);

  EXPECT_EQ(params.get<int64_t>("int_val"), 42);
  EXPECT_DOUBLE_EQ(params.get<double>("float_val"), 3.14);
  EXPECT_EQ(params.get<bool>("bool_val"), true);
  EXPECT_EQ(params.get<std::string>("string_val"), "test");
}
