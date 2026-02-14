#include "falcon-autotuner/ParameterMap.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;

TEST(ParameterMapTest, SetAndGet) {
  ParameterMap map;

  map.set("int_value", 42);
  map.set("double_value", 3.14);
  map.set("string_value", std::string("hello"));
  map.set("bool_value", true);

  EXPECT_EQ(map.get<int>("int_value"), 42);
  EXPECT_DOUBLE_EQ(map.get<double>("double_value"), 3.14);
  EXPECT_EQ(map.get<std::string>("string_value"), "hello");
  EXPECT_TRUE(map.get<bool>("bool_value"));
}

TEST(ParameterMapTest, TypeMismatchThrows) {
  ParameterMap map;
  map.set("value", 42);

  EXPECT_THROW(map.get<std::string>("value"), std::bad_any_cast);
}

TEST(ParameterMapTest, TryGet) {
  ParameterMap map;
  map.set("existing", 42);

  auto existing = map.try_get<int>("existing");
  EXPECT_TRUE(existing.has_value());
  EXPECT_EQ(*existing, 42);

  auto missing = map.try_get<int>("missing");
  EXPECT_FALSE(missing.has_value());

  auto wrong_type = map.try_get<std::string>("existing");
  EXPECT_FALSE(wrong_type.has_value());
}

TEST(ParameterMapTest, Has) {
  ParameterMap map;
  map.set("key", 42);

  EXPECT_TRUE(map.has("key"));
  EXPECT_FALSE(map.has("missing"));
}

TEST(ParameterMapTest, Remove) {
  ParameterMap map;
  map.set("key", 42);

  EXPECT_TRUE(map.has("key"));
  map.remove("key");
  EXPECT_FALSE(map.has("key"));
}

TEST(ParameterMapTest, Keys) {
  ParameterMap map;
  map.set("a", 1);
  map.set("b", 2);
  map.set("c", 3);

  auto keys = map.keys();
  EXPECT_EQ(keys.size(), 3);
  EXPECT_TRUE(std::find(keys.begin(), keys.end(), "a") != keys.end());
  EXPECT_TRUE(std::find(keys.begin(), keys.end(), "b") != keys.end());
  EXPECT_TRUE(std::find(keys.begin(), keys.end(), "c") != keys.end());
}

TEST(ParameterMapTest, Clear) {
  ParameterMap map;
  map.set("a", 1);
  map.set("b", 2);

  EXPECT_EQ(map.keys().size(), 2);
  map.clear();
  EXPECT_EQ(map.keys().size(), 0);
}

TEST(ParameterMapTest, Merge) {
  ParameterMap map1;
  map1.set("a", 1);
  map1.set("b", 2);

  ParameterMap map2;
  map2.set("b", 999); // This should NOT override
  map2.set("c", 3);

  map1.merge(map2);

  EXPECT_EQ(map1.get<int>("a"), 1);
  EXPECT_EQ(map1.get<int>("b"), 2); // Original value preserved
  EXPECT_EQ(map1.get<int>("c"), 3);
}

TEST(ParameterMapTest, ReadonlyView) {
  ParameterMap map;
  map.set("value", 42);

  auto readonly = map.create_readonly_view();

  EXPECT_TRUE(readonly.is_readonly());
  EXPECT_EQ(readonly.get<int>("value"), 42);
}

TEST(ParameterMapTest, ToJsonAndBack) {
  ParameterMap map;
  map.set("int", int64_t(42));
  map.set("double", 3.14);
  map.set("bool", true);
  map.set("string", std::string("hello"));

  auto j = map.to_json();
  auto map2 = ParameterMap::from_json(j);

  EXPECT_EQ(map2.get<int64_t>("int"), 42);
  EXPECT_DOUBLE_EQ(map2.get<double>("double"), 3.14);
  EXPECT_TRUE(map2.get<bool>("bool"));
  EXPECT_EQ(map2.get<std::string>("string"), "hello");
}
