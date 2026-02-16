#include "../test_utils/TestFixture.hpp"
#include "falcon-routine/database.hpp"
#include "falcon-routine/hub.hpp"
#include "falcon-routine/log.hpp"
#include <atomic>
#include <gtest/gtest.h>
#include <thread>

using namespace falcon::routine;
using namespace falcon::routine::test;
using namespace falcon::database;

class IntegrationTest : public DatabaseTestFixture {};

TEST_F(IntegrationTest, DatabaseAndLogging) {
  log::info("Starting integration test");

  // Insert via main connection
  DeviceCharacteristic dc;
  dc.scope = "integration_test";
  dc.name = "test_device";
  dc.characteristic = JSONPrimitive("test_value");

  db_->insert(dc);
  log::debug("Inserted test device");

  // Read via lazy connection
  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());
  auto result = lazy_db.get_by_name("test_device");

  ASSERT_TRUE(result.has_value());
  log::info("Successfully retrieved test device");

  EXPECT_EQ(result->name, "test_device");
}

TEST_F(IntegrationTest, HubAndDatabaseTogether) {
  // Store calibration data in database
  DeviceCharacteristic cal;
  cal.scope = "calibration";
  cal.name = "cal_factor";
  cal.characteristic = JSONPrimitive(1.5);
  db_->insert(cal);

  log::info("Stored calibration data");

  // Retrieve from database
  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());
  auto cal_data = lazy_db.get_by_name("cal_factor");

  ASSERT_TRUE(cal_data.has_value());

  // Publish via NATS
  Hub &hub = Hub::instance();
  nlohmann::json msg;
  msg["calibration"] = cal_data->characteristic.to_json();
  msg["timestamp"] = std::time(nullptr);

  EXPECT_NO_THROW(hub.publish_json("calibration.update", msg));

  log::info("Published calibration update");
}

TEST_F(IntegrationTest, CompleteWorkflow) {
  log::info("=== Starting Complete Workflow Test ===");

  // Step 1: Store initial configuration in database
  DeviceCharacteristic config;
  config.scope = "config";
  config.name = "measurement_params";
  nlohmann::json params;
  params["voltage_min"] = 0.0;
  params["voltage_max"] = 5.0;
  params["step"] = 0.1;
  config.characteristic = JSONPrimitive::from_json(params);
  db_->insert(config);

  log::debug("Step 1: Configuration stored");

  // Step 2: Set up NATS responder (simulates measurement service)
  std::atomic<bool> responder_ready{false};
  std::atomic<bool> measurement_received{false};

  std::thread responder([&]() {
    natsConnection *conn = nullptr;
    natsConnection_ConnectTo(&conn, getNatsUrl().c_str());

    natsSubscription *sub = nullptr;
    natsConnection_SubscribeSync(&sub, conn, "measurement.request");

    responder_ready = true;
    log::debug("Step 2: Measurement service ready");

    natsMsg *msg = nullptr;
    if (natsSubscription_NextMsg(&msg, sub, 5000) == NATS_OK) {
      measurement_received = true;

      const char *reply_to = natsMsg_GetReply(msg);
      if (reply_to) {
        nlohmann::json response;
        response["voltage"] = 2.5;
        response["current"] = 0.003;
        response["status"] = "ok";

        natsConnection_PublishString(conn, reply_to, response.dump().c_str());
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

  // Step 3: Read config from database
  LazyReadOnlyDatabaseConnection lazy_db(getDatabaseUrl());
  auto config_data = lazy_db.get_by_name("measurement_params");
  ASSERT_TRUE(config_data.has_value());

  log::debug("Step 3: Configuration retrieved");

  // Step 4: Send measurement request via NATS
  Hub &hub = Hub::instance();
  nlohmann::json request;
  request["voltage"] = 2.5;
  request["config"] = config_data->characteristic.to_json();

  auto response = hub.request_json("measurement.request", request, 2000);

  responder.join();

  ASSERT_TRUE(measurement_received);
  ASSERT_TRUE(response.has_value());

  log::debug("Step 4: Measurement response received");

  // Step 5: Store result in database
  DeviceCharacteristic result_dc;
  result_dc.scope = "results";
  result_dc.name = "measurement_1";
  result_dc.characteristic = JSONPrimitive::from_json(*response);
  db_->insert(result_dc);

  log::debug("Step 5: Result stored in database");

  // Step 6: Verify complete workflow
  auto stored_result = lazy_db.get_by_name("measurement_1");
  ASSERT_TRUE(stored_result.has_value());

  auto result_json = stored_result->characteristic.to_json();
  EXPECT_EQ(result_json["voltage"], 2.5);
  EXPECT_EQ(result_json["current"], 0.003);
  EXPECT_EQ(result_json["status"], "ok");

  log::info("=== Complete Workflow Test PASSED ===");
}
