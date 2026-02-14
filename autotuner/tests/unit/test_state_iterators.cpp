#include "falcon-autotuner/StateIterators.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;

TEST(ListIteratorTest, BasicIteration) {
  std::vector<int> values = {1, 2, 3, 4, 5};
  ListIterator<int> iter(values);

  EXPECT_EQ(iter.size(), 5);
  EXPECT_TRUE(iter.has_next());

  for (int expected = 1; expected <= 5; ++expected) {
    EXPECT_TRUE(iter.has_next());
    EXPECT_EQ(iter.next(), expected);
  }

  EXPECT_FALSE(iter.has_next());
}

TEST(ListIteratorTest, Reset) {
  std::vector<int> values = {1, 2, 3};
  ListIterator<int> iter(values);

  EXPECT_EQ(iter.next(), 1);
  EXPECT_EQ(iter.next(), 2);

  iter.reset();

  EXPECT_EQ(iter.next(), 1);
  EXPECT_EQ(iter.next(), 2);
}

TEST(IntegerRangeIteratorTest, PositiveStep) {
  IntegerRangeIterator iter(0, 10, 2);

  EXPECT_EQ(iter.size(), 5);

  std::vector<int64_t> expected = {0, 2, 4, 6, 8};
  for (auto exp : expected) {
    EXPECT_TRUE(iter.has_next());
    EXPECT_EQ(iter.next(), exp);
  }

  EXPECT_FALSE(iter.has_next());
}

TEST(IntegerRangeIteratorTest, NegativeStep) {
  IntegerRangeIterator iter(10, 0, -2);

  std::vector<int64_t> expected = {10, 8, 6, 4, 2};
  for (auto exp : expected) {
    EXPECT_TRUE(iter.has_next());
    EXPECT_EQ(iter.next(), exp);
  }

  EXPECT_FALSE(iter.has_next());
}

TEST(FloatRangeIteratorTest, EvenDivisions) {
  FloatRangeIterator iter(0.0, 1.0, 4);

  EXPECT_EQ(iter.size(), 5);

  std::vector<double> expected = {0.0, 0.25, 0.5, 0.75, 1.0};
  for (auto exp : expected) {
    EXPECT_TRUE(iter.has_next());
    EXPECT_DOUBLE_EQ(iter.next(), exp);
  }

  EXPECT_FALSE(iter.has_next());
}

TEST(GeneratorIteratorTest, CustomGenerator) {
  int count = 0;
  auto generator = [&count]() -> std::optional<int> {
    if (count < 3) {
      return count++;
    }
    return std::nullopt;
  };

  GeneratorIterator<int> iter(generator);

  EXPECT_EQ(iter.next(), 0);
  EXPECT_EQ(iter.next(), 1);
  EXPECT_EQ(iter.next(), 2);
  EXPECT_FALSE(iter.has_next());
}
