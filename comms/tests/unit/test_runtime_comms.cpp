#include "../test_utils/TestFixture.hpp"
#include "falcon-comms/runtime_comms.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::comms;
using namespace falcon::comms::test;

class RuntimeCommsUnitTest : public CommsTestFixture {};

TEST_F(RuntimeCommsUnitTest, RequestDeviceConfig) {
  RuntimeComms comms;

  // Set up mock responder
  std::atomic<bool> responder_ready{false};
  std::thread responder([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *sub = nullptr;
    natsConnection_SubscribeSync(&sub, conn,
                                 "INSTRUMENTHUB.DEVICE_CONFIG_REQUEST");

    responder_ready = true;

    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, sub, 5000) == NATS_OK) {
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        DeviceConfigResponse response;
        response.timestamp = 456789;
        response.response =
            "{\"devices\": [{\"name\": \"SMU1\", \"type\": \"voltage\"}]}";

        nlohmann::json j = response.to_json();
        natsConnection_PublishString(conn, reply_to, j.dump().c_str());
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

  auto config_response = comms.request_device_config(3000);

  responder.join();

  ASSERT_TRUE(config_response.has_value());
  EXPECT_EQ(config_response->timestamp, 456789);
  EXPECT_TRUE(config_response->response.find("SMU1") != std::string::npos);
}

TEST_F(RuntimeCommsUnitTest, RequestPorts) {
  RuntimeComms comms;

  // Set up mock responder
  std::atomic<bool> responder_ready{false};
  std::thread responder([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *sub = nullptr;
    natsConnection_SubscribeSync(&sub, conn, "INSTRUMENTHUB.PORT_REQUEST");

    responder_ready = true;

    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, sub, 5000) == NATS_OK) {
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        PortPayload payload;
        payload.timestamp = 654321;
        payload.knobs = "{\"voltage_knob\": \"SMU1.voltage\"}";
        payload.meters = "{\"current_meter\": \"SMU1.current\"}";

        nlohmann::json j = payload.to_json();
        natsConnection_PublishString(conn, reply_to, j.dump().c_str());
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

  auto port_payload = comms.request_ports(3000);

  responder.join();

  ASSERT_TRUE(port_payload.has_value());
  EXPECT_EQ(port_payload->timestamp, 654321);
  EXPECT_TRUE(port_payload->knobs.find("voltage_knob") != std::string::npos);
  EXPECT_TRUE(port_payload->meters.find("current_meter") != std::string::npos);
}

TEST_F(RuntimeCommsUnitTest, SubscribeDeviceConfigResponse) {
  RuntimeComms comms;

  std::atomic<bool> callback_called{false};
  DeviceConfigResponse received_response;

  comms.subscribe_device_config_response(
      [&](const DeviceConfigResponse &response) {
        received_response = response;
        callback_called = true;
      });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a device config response
  NatsManager &hub = NatsManager::instance();
  DeviceConfigResponse test_response;
  test_response.timestamp = 111222;
  test_response.response = "{\"config\": \"test\"}";

  hub.publish_json("INSTRUMENTHUB.DEVICE_CONFIG_RESPONSE",
                   test_response.to_json());

  // Wait for callback
  for (int i = 0; i < 50; i++) {
    if (callback_called)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  EXPECT_TRUE(callback_called);
  EXPECT_EQ(received_response.timestamp, 111222);
  EXPECT_EQ(received_response.response, "{\"config\": \"test\"}");
}

TEST_F(RuntimeCommsUnitTest, SubscribePortPayload) {
  RuntimeComms comms;

  std::atomic<bool> callback_called{false};
  PortPayload received_payload;

  comms.subscribe_port_payload([&](const PortPayload &payload) {
    received_payload = payload;
    callback_called = true;
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Publish a port payload
  NatsManager &hub = NatsManager::instance();
  PortPayload test_payload;
  test_payload.timestamp = 333444;
  test_payload.knobs = "{\"knob1\": \"value1\"}";
  test_payload.meters = "{\"meter1\": \"value1\"}";

  hub.publish_json("INSTRUMENTHUB.PORT_PAYLOAD", test_payload.to_json());

  // Wait for callback
  for (int i = 0; i < 50; i++) {
    if (callback_called)
      break;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  EXPECT_TRUE(callback_called);
  EXPECT_EQ(received_payload.timestamp, 333444);
  EXPECT_EQ(received_payload.knobs, "{\"knob1\": \"value1\"}");
  EXPECT_EQ(received_payload.meters, "{\"meter1\": \"value1\"}");
}

TEST_F(RuntimeCommsUnitTest, RequestDeviceConfigTimeout) {
  RuntimeComms comms;

  // No responder, should timeout
  auto config_response = comms.request_device_config(500);

  EXPECT_FALSE(config_response.has_value());
}

TEST_F(RuntimeCommsUnitTest, RequestPortsTimeout) {
  RuntimeComms comms;

  // No responder, should timeout
  auto port_payload = comms.request_ports(500);

  EXPECT_FALSE(port_payload.has_value());
}
