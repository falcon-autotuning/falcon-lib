#include "../test_utils/TestFixture.hpp"
#include "falcon-comms/autotuner_comms.hpp"
#include "falcon-comms/routine_comms.hpp"
#include "falcon-comms/runtime_comms.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::comms;
using namespace falcon::comms::test;

class IntegrationTest : public CommsTestFixture {};

TEST_F(IntegrationTest, AutotunerWorkflow) {
  AutotunerComms autotuner;

  std::atomic<bool> responder_ready{false};
  std::atomic<bool> request_received{false};

  // Simulate instrument hub responding to state requests
  std::thread hub_simulator([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *sub = nullptr;
    natsConnection_SubscribeSync(&sub, conn, "INSTRUMENTHUB.STATE_REQUEST");

    responder_ready = true;

    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, sub, 5000) == NATS_OK) {
      request_received = true;
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        StateResponse response;
        response.timestamp = 100000;
        response.response =
            "{\"voltage\": 1.5, \"current\": 0.002, \"status\": \"ok\"}";

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

  // Autotuner requests device state
  auto state = autotuner.request_state(3000);

  hub_simulator.join();

  EXPECT_TRUE(request_received);
  ASSERT_TRUE(state.has_value());
  EXPECT_TRUE(state->response.find("voltage") != std::string::npos);
}

TEST_F(IntegrationTest, RuntimeWorkflow) {
  RuntimeComms runtime;

  std::atomic<bool> responder_ready{false};
  std::atomic<int> requests_received{0};

  // Simulate instrument hub responding to runtime requests
  std::thread hub_simulator([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    // Subscribe to both config and port requests
    natsSubscription *config_sub = nullptr;
    natsSubscription *port_sub = nullptr;
    natsConnection_SubscribeSync(&config_sub, conn,
                                 "INSTRUMENTHUB.DEVICE_CONFIG_REQUEST");
    natsConnection_SubscribeSync(&port_sub, conn, "INSTRUMENTHUB.PORT_REQUEST");

    responder_ready = true;

    // Handle config request
    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, config_sub, 5000) == NATS_OK) {
      requests_received++;
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        DeviceConfigResponse response;
        response.timestamp = 200000;
        response.response = "{\"devices\": [\"SMU1\", \"SMU2\"]}";

        nlohmann::json j = response.to_json();
        natsConnection_PublishString(conn, reply_to, j.dump().c_str());
      }
      natsMsg_Destroy(msg);
    }

    // Handle port request
    if (natsSubscription_NextMsg(&msg, port_sub, 5000) == NATS_OK) {
      requests_received++;
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        PortPayload payload;
        payload.timestamp = 200001;
        payload.knobs = "{\"V1\": \"SMU1.voltage\"}";
        payload.meters = "{\"I1\": \"SMU1.current\"}";

        nlohmann::json j = payload.to_json();
        natsConnection_PublishString(conn, reply_to, j.dump().c_str());
      }
      natsMsg_Destroy(msg);
    }

    natsSubscription_Destroy(config_sub);
    natsSubscription_Destroy(port_sub);
    natsConnection_Destroy(conn);
  });

  while (!responder_ready) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Runtime requests device configuration
  auto config = runtime.request_device_config(3000);

  // Runtime requests port information
  auto ports = runtime.request_ports(3000);

  hub_simulator.join();

  EXPECT_EQ(requests_received, 2);
  ASSERT_TRUE(config.has_value());
  EXPECT_TRUE(config->response.find("SMU1") != std::string::npos);

  ASSERT_TRUE(ports.has_value());
  EXPECT_TRUE(ports->knobs.find("V1") != std::string::npos);
}

TEST_F(IntegrationTest, RoutineInheritanceWorkflow) {
  RoutineComms routine;

  std::atomic<bool> responder_ready{false};
  std::atomic<bool> state_request_received{false};
  std::atomic<bool> measure_request_received{false};

  // Simulate instrument hub responding to both state and measure requests
  std::thread hub_simulator([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *state_sub = nullptr;
    natsSubscription *measure_sub = nullptr;
    natsConnection_SubscribeSync(&state_sub, conn,
                                 "INSTRUMENTHUB.STATE_REQUEST");
    natsConnection_SubscribeSync(&measure_sub, conn,
                                 "INSTRUMENTHUB.MEASURE_COMMAND");

    responder_ready = true;

    // Handle state request (inherited from AutotunerComms)
    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, state_sub, 5000) == NATS_OK) {
      state_request_received = true;
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        StateResponse response;
        response.timestamp = 300000;
        response.response = "{\"ready\": true}";

        nlohmann::json j = response.to_json();
        natsConnection_PublishString(conn, reply_to, j.dump().c_str());
      }
      natsMsg_Destroy(msg);
    }

    // Handle measurement request (RoutineComms specific)
    if (natsSubscription_NextMsg(&msg, measure_sub, 5000) == NATS_OK) {
      measure_request_received = true;
      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        MeasureResponse response;
        response.timestamp = 300001;
        response.stream = "MEASUREMENTS";
        response.channel = "ch1";

        nlohmann::json j = response.to_json();
        natsConnection_PublishString(conn, reply_to, j.dump().c_str());
      }
      natsMsg_Destroy(msg);
    }

    natsSubscription_Destroy(state_sub);
    natsSubscription_Destroy(measure_sub);
    natsConnection_Destroy(conn);
  });

  while (!responder_ready) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Test inherited functionality: state request
  auto state = routine.request_state(3000);

  // Test routine-specific functionality: measurement request
  auto measurement = routine.request_measurement("test_measure", 3000);

  hub_simulator.join();

  // Verify both inherited and specific functionality work
  EXPECT_TRUE(state_request_received);
  ASSERT_TRUE(state.has_value());
  EXPECT_EQ(state->response, "{\"ready\": true}");

  EXPECT_TRUE(measure_request_received);
  ASSERT_TRUE(measurement.has_value());
  EXPECT_EQ(measurement->first.stream, "MEASUREMENTS");
}

TEST_F(IntegrationTest, AllThreeCommsSimultaneously) {
  AutotunerComms autotuner;
  RuntimeComms runtime;
  RoutineComms routine;

  std::atomic<bool> responder_ready{false};
  std::atomic<int> total_requests{0};

  // Simulate full instrument hub
  std::thread hub_simulator([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    // Subscribe to all subjects
    natsSubscription *state_sub = nullptr;
    natsSubscription *config_sub = nullptr;
    natsSubscription *port_sub = nullptr;
    natsSubscription *measure_sub = nullptr;

    natsConnection_SubscribeSync(&state_sub, conn,
                                 "INSTRUMENTHUB.STATE_REQUEST");
    natsConnection_SubscribeSync(&config_sub, conn,
                                 "INSTRUMENTHUB.DEVICE_CONFIG_REQUEST");
    natsConnection_SubscribeSync(&port_sub, conn, "INSTRUMENTHUB.PORT_REQUEST");
    natsConnection_SubscribeSync(&measure_sub, conn,
                                 "INSTRUMENTHUB.MEASURE_COMMAND");

    responder_ready = true;

    // Process requests for 5 seconds
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
      natsMsg *msg = nullptr;

      // Check state requests
      if (natsSubscription_NextMsg(&msg, state_sub, 100) == NATS_OK) {
        total_requests++;
        const char *reply_to = natsMsg_GetReply(msg);
        if (reply_to) {
          StateResponse r;
          r.timestamp = 1;
          r.response = "{}";
          natsConnection_PublishString(conn, reply_to,
                                       r.to_json().dump().c_str());
        }
        natsMsg_Destroy(msg);
      }

      // Check config requests
      if (natsSubscription_NextMsg(&msg, config_sub, 100) == NATS_OK) {
        total_requests++;
        const char *reply_to = natsMsg_GetReply(msg);
        if (reply_to) {
          DeviceConfigResponse r;
          r.timestamp = 1;
          r.response = "{}";
          natsConnection_PublishString(conn, reply_to,
                                       r.to_json().dump().c_str());
        }
        natsMsg_Destroy(msg);
      }

      // Check port requests
      if (natsSubscription_NextMsg(&msg, port_sub, 100) == NATS_OK) {
        total_requests++;
        const char *reply_to = natsMsg_GetReply(msg);
        if (reply_to) {
          PortPayload r;
          r.timestamp = 1;
          r.knobs = "{}";
          r.meters = "{}";
          natsConnection_PublishString(conn, reply_to,
                                       r.to_json().dump().c_str());
        }
        natsMsg_Destroy(msg);
      }

      // Check measure requests
      if (natsSubscription_NextMsg(&msg, measure_sub, 100) == NATS_OK) {
        total_requests++;
        const char *reply_to = natsMsg_GetReply(msg);
        if (reply_to) {
          MeasureResponse r;
          r.timestamp = 1;
          r.stream = "TEST";
          r.channel = "ch1";
          natsConnection_PublishString(conn, reply_to,
                                       r.to_json().dump().c_str());
        }
        natsMsg_Destroy(msg);
      }
    }

    natsSubscription_Destroy(state_sub);
    natsSubscription_Destroy(config_sub);
    natsSubscription_Destroy(port_sub);
    natsSubscription_Destroy(measure_sub);
    natsConnection_Destroy(conn);
  });

  while (!responder_ready) {
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  // Make requests from all three comm services
  auto state = autotuner.request_state(2000);
  auto config = runtime.request_device_config(2000);
  auto ports = runtime.request_ports(2000);
  auto measure = routine.request_measurement("test", 2000);

  hub_simulator.join();

  // Verify all requests were handled
  EXPECT_GE(total_requests, 4);
  EXPECT_TRUE(state.has_value());
  EXPECT_TRUE(config.has_value());
  EXPECT_TRUE(ports.has_value());
  EXPECT_TRUE(measure.has_value());
}
