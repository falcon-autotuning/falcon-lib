#include "../test_utils/TestFixture.hpp"
#include "falcon-comms/commands_definitions.hpp"
#include "falcon-comms/runtime_comms.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::comms;
using namespace falcon::comms::test;

class RuntimeCommsUnitTest : public CommsTestFixture {};

TEST_F(RuntimeCommsUnitTest, SubscribeConfigResponse) {
  using falcon::comms::NatsManager;
  RuntimeComms comms;
  std::atomic<bool> responder_ready{false};
  std::atomic<bool> request_received{false};
  std::optional<DeviceConfigResponse> response;

  // Start responder thread
  std::thread responder([&]() {
    auto &hub = NatsManager::instance();
    hub.subscribe(
        "INSTRUMENTHUB.DEVICE_CONFIG_REQUEST", [&](const std::string &msg) {
          request_received = true;
          DeviceConfigResponse response;
          response.timestamp = 456789;
          response.response =
              "{\"devices\": [{\"name\": \"SMU1\", \"type\": \"voltage\"}]}";

          nlohmann::json j = response.to_json();
          hub.publish("FALCON.DEVICE_CONFIG_RESPONSE", j.dump());
        });
    responder_ready = true;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    hub.unsubscribe("INSTRUMENTHUB.DEVICE_CONFIG_REQUEST");
  });
  while (!responder_ready)
    std::this_thread::yield();
  std::thread client([&]() {
    try {
      response = comms.subscribe_config_response(3000, 100000);
    } catch (const std::exception &e) {
      std::cout << "EXCEPTION in client thread: " << e.what() << std::endl;
      throw;
    }
  });
  client.join();
  responder.join();

  EXPECT_TRUE(request_received);
  EXPECT_EQ(response->timestamp, 456789);
  EXPECT_TRUE(response->response.find("SMU1") != std::string::npos);
}

TEST_F(RuntimeCommsUnitTest, SubscribePortPayload) {
  using falcon::comms::NatsManager;
  RuntimeComms comms;
  std::atomic<bool> responder_ready{false};
  std::atomic<bool> request_received{false};
  std::optional<PortPayload> response;

  // Start responder thread
  std::thread responder([&]() {
    auto &hub = NatsManager::instance();
    hub.subscribe("INSTRUMENTHUB.PORT_REQUEST", [&](const std::string &msg) {
      request_received = true;
      PortPayload payload;
      payload.timestamp = 654321;
      payload.knobs = "{\"voltage_knob\": \"SMU1.voltage\"}";
      payload.meters = "{\"current_meter\": \"SMU1.current\"}";
      nlohmann::json j = payload.to_json();
      hub.publish("FALCON.PORT_PAYLOAD", j.dump());
    });
    responder_ready = true;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    hub.unsubscribe("INSTRUMENTHUB.PORT_REQUEST");
  });
  while (!responder_ready)
    std::this_thread::yield();
  std::thread client([&]() {
    try {
      response = comms.subscribe_port_payload(3000, 100000);
    } catch (const std::exception &e) {
      std::cout << "EXCEPTION in client thread: " << e.what() << std::endl;
      throw;
    }
  });
  client.join();
  responder.join();

  EXPECT_TRUE(request_received);
  EXPECT_EQ(response->timestamp, 654321);
  EXPECT_TRUE(response->knobs.find("voltage_knob") != std::string::npos);
  EXPECT_TRUE(response->meters.find("current_meter") != std::string::npos);
}

TEST_F(RuntimeCommsUnitTest, SubscribeConfigResponseTimeout) {
  RuntimeComms comms;

  // No responder, should timeout and throw
  EXPECT_THROW(comms.subscribe_config_response(500, 300000),
               std::runtime_error);
}

TEST_F(RuntimeCommsUnitTest, SubscribePortPayloadTimeout) {
  RuntimeComms comms;

  // No responder, should timeout and throw
  EXPECT_THROW(comms.subscribe_port_payload(500, 400000), std::runtime_error);
}
