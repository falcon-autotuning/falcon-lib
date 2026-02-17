#include "falcon-comms/runtime_comms.hpp"
#include "falcon-comms/commands_definitions.hpp"
#include <chrono>
#include <future>
#include <spdlog/spdlog.h>

namespace {
std::string make_port_request_subject() {
  return "INSTRUMENTHUB." + std::string(PortRequest::NAME);
}
std::string make_port_response_subject() {
  return "FALCON." + std::string(PortPayload::NAME);
}
std::string make_config_request_subject() {
  return "INSTRUMENTHUB." + std::string(DeviceConfigRequest::NAME);
}
std::string make_config_response_subject() {
  return "FALCON." + std::string(DeviceConfigResponse::NAME);
}
} // namespace
namespace falcon::comms {

RuntimeComms::RuntimeComms() : hub_(NatsManager::instance()) {}

DeviceConfigResponse RuntimeComms::subscribe_config_response(int timeout_ms,
                                                             int time) {
  std::promise<DeviceConfigResponse> prom;
  auto fut = prom.get_future();

  // Subscribe with a one-shot callback
  hub_.subscribe(make_config_response_subject(), [&prom](
                                                     const std::string &data) {
    try {
      auto json = nlohmann::json::parse(data);
      DeviceConfigResponse response = DeviceConfigResponse::from_json(json);
      prom.set_value(response);
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse DeviceConfigResponse in subscription: {}",
                    e.what());
    }
  });

  StateRequest req;
  req.timestamp = time;

  hub_.publish(make_config_request_subject(), req.to_json());
  // Wait for the response or timeout
  if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
      std::future_status::ready) {
    // Optionally unsubscribe here if your hub supports it, using sub_id
    return fut.get();
  } // Optionally unsubscribe here if your hub supports it, using sub_id
  throw std::runtime_error("Timeout waiting for DeviceConfigResponse");
}

PortPayload RuntimeComms::subscribe_port_payload(int timeout_ms, int time) {
  std::promise<PortPayload> prom;
  auto fut = prom.get_future();

  // Subscribe with a one-shot callback
  hub_.subscribe(
      make_config_response_subject(), [&prom](const std::string &data) {
        try {
          auto json = nlohmann::json::parse(data);
          PortPayload response = PortPayload::from_json(json);
          prom.set_value(response);
        } catch (const std::exception &e) {
          spdlog::error("Failed to parse PortPayload in subscription: {}",
                        e.what());
        }
      });

  StateRequest req;
  req.timestamp = time;

  hub_.publish(make_config_request_subject(), req.to_json());
  // Wait for the response or timeout
  if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
      std::future_status::ready) {
    // Optionally unsubscribe here if your hub supports it, using sub_id
    return fut.get();
  } // Optionally unsubscribe here if your hub supports it, using sub_id
  throw std::runtime_error("Timeout waiting for PortPayload");
}

} // namespace falcon::comms
