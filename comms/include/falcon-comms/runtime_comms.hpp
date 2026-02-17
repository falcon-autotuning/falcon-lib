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
   * @brief Subscribe to device config with a callback
   * @param timeout_ms the timeout in milliseconds to wait
   * @param time current time
   * @return DeviceConfigResponse if successful
   */
  DeviceConfigResponse subscribe_config_response(int timeout_ms, int time);

  /**
   * @brief Subscribe to ports with a callback
   * @param timeout_ms the timeout in milliseconds to wait
   * @param time current time
   * @return  PortPayloadif successful
   */
  PortPayload subscribe_port_payload(int timeout_ms, int time);

private:
  NatsManager &hub_;
};

} // namespace falcon::comms
