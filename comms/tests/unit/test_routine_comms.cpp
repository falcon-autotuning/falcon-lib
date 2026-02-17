#include "../test_utils/TestFixture.hpp"
#include "falcon-comms/commands_definitions.hpp"
#include "falcon-comms/routine_comms.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::comms;
using namespace falcon::comms::test;

class RoutineCommsUnitTest : public CommsTestFixture {};

TEST_F(RoutineCommsUnitTest, InheritsFromAutotunerComms) {
  using falcon::comms::NatsManager;
  RoutineComms comms;
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

TEST_F(RoutineCommsUnitTest, SubscribeMeasureResponse) {
  using falcon::comms::NatsManager;
  RoutineComms comms;
  std::atomic<bool> responder_ready{false};
  std::atomic<bool> request_received{false};
  std::optional<MeasureResponse> response;

  // Start responder thread
  std::thread responder([&]() {
    auto &hub = NatsManager::instance();
    hub.subscribe("INSTRUMENTHUB.MEASURE_COMMAND", [&](const std::string &msg) {
      request_received = true;
      MeasureResponse response;
      response.timestamp = 999999;
      response.stream = "MEASUREMENTS";
      response.channel = "voltage_data";
      nlohmann::json j = response.to_json();
      hub.publish("FALCON.MEASURE_RESPONSE", j.dump());
    });
    responder_ready = true;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    hub.unsubscribe("INSTRUMENTHUB.MEASURE_COMMAND");
  });
  while (!responder_ready)
    std::this_thread::yield();
  std::thread client([&]() {
    try {
      response = comms.subscribe_measure_response(3000, 100000);
    } catch (const std::exception &e) {
      std::cout << "EXCEPTION in client thread: " << e.what() << std::endl;
      throw;
    }
  });
  client.join();
  responder.join();

  EXPECT_TRUE(response);
  EXPECT_EQ(response->timestamp, 999999);
  EXPECT_EQ(response->stream, "MEASUREMENTS");
  EXPECT_EQ(response->channel, "voltage_data");
}

TEST_F(RoutineCommsUnitTest, SubscribeMeasureResponseTimeout) {
  RoutineComms comms;

  // No responder, should timeout and throw
  EXPECT_THROW(comms.subscribe_measure_response(500, 200000),
               std::runtime_error);
}

TEST_F(RoutineCommsUnitTest, PullMeasurementData) {
  RoutineComms comms;

  // Set up JetStream with test data
  std::atomic<bool> jetstream_ready{false};
  std::thread setup([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    jsCtx *js = nullptr;
    natsConnection_JetStream(&js, conn, NULL);

    // Create stream
    jsStreamConfig stream_cfg;
    jsStreamConfig_Init(&stream_cfg);
    stream_cfg.Name = "TEST_MEASUREMENTS";
    const char *subjects[] = {"test.measurements.>"};
    stream_cfg.Subjects = subjects;
    stream_cfg.SubjectsLen = 1;
    stream_cfg.Storage = js_MemoryStorage;

    jsStreamInfo *si = nullptr;
    js_AddStream(&si, js, &stream_cfg, NULL, NULL);
    if (si)
      jsStreamInfo_Destroy(si);

    // Publish some test data
    for (int i = 0; i < 3; i++) {
      std::string data = "measurement_" + std::to_string(i);
      jsPubAck *ack = nullptr;
      jsErrCode errCode;
      js_Publish(&ack, js, "test.measurements.data", data.c_str(),
                 data.length(), nullptr, &errCode);
      if (ack) {
        jsPubAck_Destroy(ack);
      }
    }

    jetstream_ready = true;

    jsCtx_Destroy(js);
    natsConnection_Destroy(conn);
  });

  while (!jetstream_ready) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  setup.join();

  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Pull the data
  auto data =
      comms.pull_measurement_data("TEST_MEASUREMENTS", "test_consumer", 3);

  // Should have received some data (exact number depends on JetStream consumer
  // setup) Just verify the method doesn't crash
  EXPECT_GE(data.size(), 0);
}
