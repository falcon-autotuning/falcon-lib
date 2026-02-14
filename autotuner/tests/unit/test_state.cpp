#include "falcon-autotuner/State.hpp"
#include <gtest/gtest.h>

using namespace falcon::autotuner;

TEST(StateTest, LocalParameters) {
  State state("test_state");

  state.local_params().set("local", 42);

  EXPECT_EQ(state.local_params().get<int>("local"), 42);
}

TEST(StateTest, ParentParameters) {
  ParameterMap parent;
  parent.set("parent_value", 100);

  State state("test_state");
  state.set_parent_params(parent);

  EXPECT_EQ(state.parent_params().get<int>("parent_value"), 100);
}

TEST(StateTest, CombinedParameters) {
  ParameterMap parent;
  parent.set("shared", 100);
  parent.set("parent_only", 200);

  State state("test_state");
  state.set_parent_params(parent);
  state.local_params().set("shared", 42); // Shadows parent
  state.local_params().set("local_only", 300);

  auto combined = state.get_combined_params();

  EXPECT_EQ(combined.get<int>("shared"), 42); // Local shadows parent
  EXPECT_EQ(combined.get<int>("parent_only"), 200);
  EXPECT_EQ(combined.get<int>("local_only"), 300);
}

TEST(StateTest, ParentIsReadonly) {
  ParameterMap parent;
  parent.set("value", 100);

  State state("test_state");
  state.set_parent_params(parent);

  EXPECT_TRUE(state.parent_params().is_readonly());
}
