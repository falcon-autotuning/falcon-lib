#include "falcon-comms/runtime_comms.hpp"
#include <chrono>
#include <spdlog/spdlog.h>

namespace falcon::comms {

RuntimeComms::RuntimeComms() : hub_(NatsManager::instance()) {}

std::optional<DeviceConfigResponse>
RuntimeComms::request_device_config(int timeout_ms) {
  // Create DeviceConfigRequest
  DeviceConfigRequest req;
  req.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();

  // Publish request and wait for response
  auto response = hub_.request_json(DEVICE_CONFIG_REQUEST_SUBJECT,
                                    req.to_json(), timeout_ms);

  if (!response) {
    spdlog::warn("Device config request timed out");
    return std::nullopt;
  }

  try {
    return DeviceConfigResponse::from_json(*response);
  } catch (const std::exception &e) {
    spdlog::error("Failed to parse DeviceConfigResponse: {}", e.what());
    return std::nullopt;
  }
}

std::optional<PortPayload> RuntimeComms::request_ports(int timeout_ms) {
  // Create PortRequest
  PortRequest req;
  req.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();

  // Publish request and wait for response
  auto response =
      hub_.request_json(PORT_REQUEST_SUBJECT, req.to_json(), timeout_ms);

  if (!response) {
    spdlog::warn("Port request timed out");
    return std::nullopt;
  }

  try {
    return PortPayload::from_json(*response);
  } catch (const std::exception &e) {
    spdlog::error("Failed to parse PortPayload: {}", e.what());
    return std::nullopt;
  }
}

void RuntimeComms::subscribe_device_config_response(
    std::function<void(const DeviceConfigResponse &)> callback) {
  hub_.subscribe(
      DEVICE_CONFIG_RESPONSE_SUBJECT, [callback](const std::string &data) {
        try {
          auto json = nlohmann::json::parse(data);
          DeviceConfigResponse response = DeviceConfigResponse::from_json(json);
          callback(response);
        } catch (const std::exception &e) {
          spdlog::error("Failed to parse DeviceConfigResponse in "
                        "subscription: {}",
                        e.what());
        }
      });
}

void RuntimeComms::subscribe_port_payload(
    std::function<void(const PortPayload &)> callback) {
  hub_.subscribe(PORT_PAYLOAD_SUBJECT, [callback](const std::string &data) {
    try {
      auto json = nlohmann::json::parse(data);
      PortPayload payload = PortPayload::from_json(json);
      callback(payload);
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse PortPayload in subscription: {}",
                    e.what());
    }
  });
}

} // namespace falcon::comms
