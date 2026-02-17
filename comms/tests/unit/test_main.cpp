#include "../test_utils/TestFixture.hpp"
#include <gtest/gtest.h>

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);

  // Register global test environment for NATS cleanup
  ::testing::AddGlobalTestEnvironment(
      new falcon::comms::test::NatsTestEnvironment());

  return RUN_ALL_TESTS();
}
