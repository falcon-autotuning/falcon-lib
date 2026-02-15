#include "falcon-autotuner/StateNode.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;

TEST(StateNodeTest, BasicConstruction) {
  StateId id("TestAutotuner", "test_state");
  StateNode node(id);

  EXPECT_EQ(node.id().autotuner_name, "TestAutotuner");
  EXPECT_EQ(node.id().state_name, "test_state");
  EXPECT_TRUE(node.is_terminal());
}

TEST(StateNodeTest, AddTransition) {
  StateId id("Test", "state1");
  StateNode node(id);

  StateTransition trans(StateId("Test", "state2"),
                        TransitionCondition::always(), 0);

  node.add_transition(std::move(trans));

  EXPECT_FALSE(node.is_terminal());
  EXPECT_EQ(node.transitions().size(), 1);
}

TEST(StateNodeTest, EvaluateAlwaysTransition) {
  StateId id("Test", "state1");
  StateNode node(id);

  node.add_transition(StateTransition(StateId("Test", "state2"),
                                      TransitionCondition::always(), 0));

  ParameterMap params;
  auto result = node.evaluate_transitions(params);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->state_name, "state2");
}

TEST(StateNodeTest, ConditionalTransition) {
  StateId id("Test", "state1");
  StateNode node(id);

  node.add_transition(
      StateTransition(StateId("Test", "high"),
                      TransitionCondition::greater_than("value", 10), 10));

  node.add_transition(StateTransition(StateId("Test", "low"),
                                      TransitionCondition::always(), 0));

  // Test high condition
  ParameterMap params1;
  params1.set("value", 15);
  auto result1 = node.evaluate_transitions(params1);
  ASSERT_TRUE(result1.has_value());
  EXPECT_EQ(result1->state_name, "high");

  // Test low condition (default)
  ParameterMap params2;
  params2.set("value", 5);
  auto result2 = node.evaluate_transitions(params2);
  ASSERT_TRUE(result2.has_value());
  EXPECT_EQ(result2->state_name, "low");
}

TEST(StateNodeTest, TransitionPriority) {
  StateId id("Test", "state1");
  StateNode node(id);

  // Add transitions in wrong order
  node.add_transition(StateTransition(StateId("Test", "low_priority"),
                                      TransitionCondition::always(), 0));

  node.add_transition(StateTransition(StateId("Test", "high_priority"),
                                      TransitionCondition::always(), 100));

  // Should evaluate high priority first
  ParameterMap params;
  auto result = node.evaluate_transitions(params);

  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result->state_name, "high_priority");
}

TEST(TransitionConditionTest, Equals) {
  auto cond = TransitionCondition::equals("x", 42);

  ParameterMap params1;
  params1.set("x", int64_t(42));
  EXPECT_TRUE(cond.evaluate(params1));

  ParameterMap params2;
  params2.set("x", int64_t(100));
  EXPECT_FALSE(cond.evaluate(params2));
}

TEST(TransitionConditionTest, GreaterThan) {
  auto cond = TransitionCondition::greater_than("value", 10.0);

  ParameterMap params1;
  params1.set("value", 15.0);
  EXPECT_TRUE(cond.evaluate(params1));

  ParameterMap params2;
  params2.set("value", 5.0);
  EXPECT_FALSE(cond.evaluate(params2));
}

TEST(TransitionConditionTest, LessThan) {
  auto cond = TransitionCondition::less_than("temp", 100.0);

  ParameterMap params1;
  params1.set("temp", 50.0);
  EXPECT_TRUE(cond.evaluate(params1));

  ParameterMap params2;
  params2.set("temp", 150.0);
  EXPECT_FALSE(cond.evaluate(params2));
}

TEST(TransitionConditionTest, ParamExists) {
  auto cond = TransitionCondition::param_exists("optional");

  ParameterMap params1;
  params1.set("optional", 42);
  EXPECT_TRUE(cond.evaluate(params1));

  ParameterMap params2;
  EXPECT_FALSE(cond.evaluate(params2));
}
