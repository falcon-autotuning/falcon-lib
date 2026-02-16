#include "../test_utils/TestFixture.hpp"
#include "falcon-comms/autotuner_comms.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::comms;
using namespace falcon::comms::test;

class AutotunerCommsUnitTest : public CommsTestFixture {};

TEST_F(AutotunerCommsUnitTest, RequestState) {
  AutotunerComms comms;

  // Set up mock responder
  std::atomic<bool> responder_ready{false};
  std::thread responder([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *sub = nullptr;
    natsConnection_SubscribeSync(&sub, conn, "INSTRUMENTHUB.STATE_REQUEST");

    responder_ready = true;

    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, sub, 5000) == NATS_OK) {
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        StateResponse response;
        response.timestamp = 123456;
        response.response = "{\"voltage\": 2.5, \"current\": 0.01}";

        nlohmann::json j = response.to_json();
        natsConnection_PublishString(conn, reply_to, j.dump().c_str());
      }
      natsMsg_Destroy(msg);
    }

    natsSubscription_Destroy(sub);
    natsConnection_Destroy(conn);
  });

  // Wait for responder
  while (!responder_ready) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Request state
  auto state_response = comms.request_state(3000);

  responder.join();

  ASSERT_TRUE(state_response.has_value());
  EXPECT_EQ(state_response->timestamp, 123456);
  EXPECT_EQ(state_response->response, "{\"voltage\": 2.5, \"current\": 0.01}");
}

TEST_F(AutotunerCommsUnitTest, SubscribeStateResponse) {
  AutotunerComms comms;

  std::atomic<bool> callback_called{false};
  StateResponse received_response;

  comms.subscribe_state_response([&](const StateResponse &response) {
    received_response = response;
    callback_called = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a state response
  NatsManager &hub = NatsManager::instance();
  StateResponse test_response;
  test_response.timestamp = 789012;
  test_response.response = "{\"test\": \"data\"}";

  hub.publish_json("INSTRUMENTHUB.STATE_RESPONSE", test_response.to_json());

  // Wait for callback
  for (int i = 0; i < 50; i++) {
    if (callback_called)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  EXPECT_TRUE(callback_called);
  EXPECT_EQ(received_response.timestamp, 789012);
  EXPECT_EQ(received_response.response, "{\"test\": \"data\"}");
}

TEST_F(AutotunerCommsUnitTest, RequestStateTimeout) {
  AutotunerComms comms;

  // No responder, should timeout
  auto state_response = comms.request_state(500);

  EXPECT_FALSE(state_response.has_value());
}
