#include "../test_utils/TestFixture.hpp"
#include "falcon-routine/hub.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::routine;
using namespace falcon::routine::test;

class HubUnitTest : public RoutineTestFixture {};

TEST_F(HubUnitTest, LazyConnection) {
  Hub &hub = Hub::instance();

  // Should not be connected initially
  // (Can't directly check, but should connect on first use)

  // First operation should connect
  hub.publish("test.subject", "test message");

  EXPECT_TRUE(hub.is_connected());
}

TEST_F(HubUnitTest, PublishString) {
  Hub &hub = Hub::instance();

  // Should not throw
  EXPECT_NO_THROW(hub.publish("test.publish", "Hello NATS"));
}

TEST_F(HubUnitTest, PublishJson) {
  Hub &hub = Hub::instance();

  nlohmann::json data;
  data["key"] = "value";
  data["number"] = 42;

  EXPECT_NO_THROW(hub.publish_json("test.json", data));
}

TEST_F(HubUnitTest, RequestReplyWithResponder) {
  Hub &hub = Hub::instance();

  // Set up a responder
  std::atomic<bool> responder_ready{false};
  std::thread responder([&]() {
    Hub &responder_hub = Hub::instance();

    // Subscribe and respond
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

TEST_F(HubUnitTest, RequestTimeout) {
  EXPECT_THROW(
      {
        Hub &hub = Hub::instance();
        hub.request("nonexistent.subject", "request", 500);
      },
      std::runtime_error); // or the specific exception type thrown
}

TEST_F(HubUnitTest, RequestReplyJson) {
  Hub &hub = Hub::instance();

  // Set up JSON responder
  std::atomic<bool> responder_ready{false};
  std::thread responder([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *sub = nullptr;
    natsConnection_SubscribeSync(&sub, conn, "test.json.request");

    responder_ready = true;

    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, sub, 5000) == NATS_OK) {
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        nlohmann::json response;
        response["status"] = "ok";
        response["value"] = 42;
        natsConnection_PublishString(conn, reply_to, response.dump().c_str());
      }
      natsMsg_Destroy(msg);
    }

    natsSubscription_Destroy(sub);
    natsConnection_Destroy(conn);
  });

  while (!responder_ready) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  nlohmann::json request;
  request["action"] = "get_value";

  auto response = hub.request_json("test.json.request", request, 2000);

  responder.join();

  ASSERT_TRUE(response.has_value());
  EXPECT_EQ((*response)["status"], "ok");
  EXPECT_EQ((*response)["value"], 42);
}

TEST_F(HubUnitTest, Subscribe) {
  Hub &hub = Hub::instance();

  std::atomic<bool> message_received{false};
  std::string received_data;

  // Subscribe
  hub.subscribe("test.subscribe", [&](const std::string &msg) {
    received_data = msg;
    message_received = true;
  });

  // Give subscription time to register
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish to the subject
  hub.publish("test.subscribe", "test_payload");

  // Wait for message
  auto start = std::chrono::steady_clock::now();
  while (!message_received &&
         std::chrono::steady_clock::now() - start < std::chrono::seconds(2)) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  EXPECT_TRUE(message_received);
  EXPECT_EQ(received_data, "test_payload");
}
