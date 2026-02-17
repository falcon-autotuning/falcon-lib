#include "../test_utils/TestFixture.hpp"
#include "falcon-comms/autotuner_comms.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <spdlog/spdlog.h>
#include <thread>

using namespace falcon::comms;
using namespace falcon::comms::test;

class AutotunerCommsUnitTest : public CommsTestFixture {};

TEST_F(AutotunerCommsUnitTest, SubscribeStateResponse) {
  using falcon::comms::NatsManager;
  AutotunerComms comms;
  std::atomic<bool> responder_ready{false};
  std::atomic<bool> request_received{false};
  std::optional<StateResponse> state_response;

  // Start responder thread
  std::thread responder([&]() {
    auto &hub = NatsManager::instance();
    hub.subscribe("INSTRUMENTHUB.STATE_REQUEST", [&](const std::string &msg) {
      request_received = true;
      StateResponse response;
      response.timestamp = 123456;
      response.response = "string";
      nlohmann::json j = response.to_json();
      hub.publish("FALCON.STATE_RESPONSE", j.dump());
    });
    responder_ready = true;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    hub.unsubscribe("INSTRUMENTHUB.STATE_REQUEST");
  });
  while (!responder_ready)
    std::this_thread::yield();
  std::thread client([&]() {
    try {
      state_response = comms.subscribe_state_response(3000, 100000);
    } catch (const std::exception &e) {
      std::cout << "EXCEPTION in client thread: " << e.what() << std::endl;
      throw;
    }
  });
  client.join();
  responder.join();

  ASSERT_TRUE(request_received);
  ASSERT_TRUE(state_response.has_value());
  EXPECT_EQ(state_response->timestamp, 123456);
  EXPECT_EQ(state_response->response, "string");
}

TEST_F(AutotunerCommsUnitTest, SubscribeStateResponseTimeout) {
  AutotunerComms comms;

  // No responder, should timeout and throw
  EXPECT_THROW(comms.subscribe_state_response(500, 100000), std::runtime_error);
}
