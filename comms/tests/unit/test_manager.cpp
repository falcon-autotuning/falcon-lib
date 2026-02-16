#include "../test_utils/TestFixture.hpp"
#include "falcon-comms/natsManager.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::comms;
using namespace falcon::comms::test;

class NatsManagerUnitTest : public CommsTestFixture {};

TEST_F(NatsManagerUnitTest, LazyConnection) {
  NatsManager &hub = NatsManager::instance();

  // First operation should connect
  hub.publish("test.subject", "test message");

  EXPECT_TRUE(hub.is_connected());
}

TEST_F(NatsManagerUnitTest, PublishString) {
  NatsManager &hub = NatsManager::instance();

  EXPECT_NO_THROW(hub.publish("test.publish", "Hello NATS"));
}

TEST_F(NatsManagerUnitTest, PublishJson) {
  NatsManager &hub = NatsManager::instance();

  nlohmann::json data;
  data["key"] = "value";
  data["number"] = 42;

  EXPECT_NO_THROW(hub.publish_json("test.json", data));
}

TEST_F(NatsManagerUnitTest, RequestReplyWithResponder) {
  NatsManager &hub = NatsManager::instance();

  // Set up a responder
  std::atomic<bool> responder_ready{false};
  std::thread responder([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *sub = nullptr;
    natsConnection_SubscribeSync(&sub, conn, "test.request");

    responder_ready = true;

    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, sub, 5000) == NATS_OK) {
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        natsConnection_PublishString(conn, reply_to, "response_data");
      }
      natsMsg_Destroy(msg);
    }

    natsSubscription_Destroy(sub);
    natsConnection_Destroy(conn);
  });

  // Wait for responder to be ready
  while (!responder_ready) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Send request
  auto response = hub.request("test.request", "request_data", 2000);

  responder.join();

  ASSERT_TRUE(response.has_value());
  EXPECT_EQ(*response, "response_data");
}

TEST_F(NatsManagerUnitTest, Subscribe) {
  NatsManager &hub = NatsManager::instance();

  std::atomic<bool> message_received{false};
  std::string received_data;

  hub.subscribe("test.subscribe", [&](const std::string &data) {
    received_data = data;
    message_received = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  hub.publish("test.subscribe", "test_message");

  // Wait for message to be received
  for (int i = 0; i < 50; i++) {
    if (message_received)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  EXPECT_TRUE(message_received);
  EXPECT_EQ(received_data, "test_message");
}
