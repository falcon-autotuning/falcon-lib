#include "TestMeasurements.hpp"
#include "falcon-autotuner/Autotuner.hpp"
#include "falcon-autotuner/GraphAnalyzer.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;
using namespace falcon::autotuner::test;

TEST(GraphAnalyzerTest, SimpleGraph) {
  auto autotuner = std::make_shared<Autotuner>("simple");
  auto measurement = std::make_shared<CountingMeasurement>();
  autotuner->set_root(Autotuner::measurement(measurement));

  auto analysis = GraphAnalyzer::analyze(*autotuner);

  EXPECT_TRUE(analysis.is_valid());
  EXPECT_FALSE(analysis.has_cycles);
  EXPECT_EQ(analysis.total_nodes, 1);
  EXPECT_EQ(analysis.max_depth, 0);
}

TEST(GraphAnalyzerTest, IteratorGraph) {
  auto autotuner = std::make_shared<Autotuner>("iterator");
  auto measurement = std::make_shared<CountingMeasurement>();

  auto root = Autotuner::iterate_range("i", 0, 10, 1,
                                       Autotuner::measurement(measurement));

  autotuner->set_root(root);

  auto analysis = GraphAnalyzer::analyze(*autotuner);

  EXPECT_TRUE(analysis.is_valid());
  EXPECT_FALSE(analysis.has_cycles);
  EXPECT_EQ(analysis.total_nodes, 2); // Iterator + Measurement
  EXPECT_EQ(analysis.max_depth, 1);
}

TEST(GraphAnalyzerTest, NestedGraph) {
  auto inner = std::make_shared<Autotuner>("inner");
  auto measurement = std::make_shared<CountingMeasurement>();
  inner->set_root(Autotuner::measurement(measurement));

  auto outer = std::make_shared<Autotuner>("outer");
  outer->set_root(Autotuner::nested(inner));

  auto analysis = GraphAnalyzer::analyze(*outer);

  EXPECT_TRUE(analysis.is_valid());
  EXPECT_FALSE(analysis.has_cycles);
  EXPECT_GE(analysis.total_nodes, 2);
}

TEST(GraphAnalyzerTest, ComplexGraph) {
  auto autotuner = std::make_shared<Autotuner>("complex");
  auto measurement = std::make_shared<CountingMeasurement>();

  // Nested iterators
  auto inner = Autotuner::iterate_range("j", 0, 5, 1,
                                        Autotuner::measurement(measurement));

  auto outer = Autotuner::iterate_range("i", 0, 3, 1, inner);

  autotuner->set_root(outer);

  auto analysis = GraphAnalyzer::analyze(*autotuner);

  EXPECT_TRUE(analysis.is_valid());
  EXPECT_FALSE(analysis.has_cycles);
  EXPECT_EQ(analysis.max_depth, 2);
}

TEST(GraphAnalyzerTest, ToDotGeneration) {
  auto autotuner = std::make_shared<Autotuner>("test");
  auto measurement = std::make_shared<CountingMeasurement>();
  autotuner->set_root(Autotuner::measurement(measurement));

  std::string dot = GraphAnalyzer::to_dot(*autotuner);

  EXPECT_FALSE(dot.empty());
  EXPECT_NE(dot.find("digraph"), std::string::npos);
  EXPECT_NE(dot.find("test"), std::string::npos);
}
