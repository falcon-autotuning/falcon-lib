#pragma once

#include "commands_definitions.hpp"
#include "natsManager.hpp"
#include <functional>
#include <optional>

namespace falcon::comms {

/**
 * @brief Communication service for Runtime operations
 *
 * Handles DeviceConfigRequest, DeviceConfigResponse, PortRequest, and
 * PortPayload.
 */
class RuntimeComms {
public:
  RuntimeComms();
  ~RuntimeComms() = default;

  /**
   * @brief Request device configuration from instrument hub
   * @param timeout_ms Timeout in milliseconds
   * @return DeviceConfigResponse if successful, nullopt otherwise
   */
  std::optional<DeviceConfigResponse>
  request_device_config(int timeout_ms = 5000);

  /**
   * @brief Request port information from instrument hub
   * @param timeout_ms Timeout in milliseconds
   * @return PortPayload if successful, nullopt otherwise
   */
  std::optional<PortPayload> request_ports(int timeout_ms = 5000);

  /**
   * @brief Subscribe to device config responses with a callback
   * @param callback Function to call when device config response received
   */
  void subscribe_device_config_response(
      std::function<void(const DeviceConfigResponse &)> callback);

  /**
   * @brief Subscribe to port payloads with a callback
   * @param callback Function to call when port payload received
   */
  void
  subscribe_port_payload(std::function<void(const PortPayload &)> callback);

private:
  NatsManager &hub_;

  // Subject constants
  static constexpr const char *DEVICE_CONFIG_REQUEST_SUBJECT =
      "INSTRUMENTHUB.DEVICE_CONFIG_REQUEST";
  static constexpr const char *DEVICE_CONFIG_RESPONSE_SUBJECT =
      "INSTRUMENTHUB.DEVICE_CONFIG_RESPONSE";
  static constexpr const char *PORT_REQUEST_SUBJECT =
      "INSTRUMENTHUB.PORT_REQUEST";
  static constexpr const char *PORT_PAYLOAD_SUBJECT =
      "INSTRUMENTHUB.PORT_PAYLOAD";
};

} // namespace falcon::comms
