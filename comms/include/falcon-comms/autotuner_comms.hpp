#pragma once

#include "commands_definitions.hpp"
#include "natsManager.hpp"
#include <functional>
#include <memory>
#include <optional>

namespace falcon::comms {

/**
 * @brief Communication service for Autotuner
 *
 * Handles StateRequest and StateResponse for autotuning operations.
 */
class AutotunerComms {
public:
  AutotunerComms();
  virtual ~AutotunerComms() = default;

  /**
   * @brief Request device state from instrument hub
   * @param timeout_ms Timeout in milliseconds
   * @return StateResponse if successful, nullopt otherwise
   */
  std::optional<StateResponse> request_state(int timeout_ms = 5000);

  /**
   * @brief Subscribe to state responses with a callback
   * @param callback Function to call when state response received
   */
  void
  subscribe_state_response(std::function<void(const StateResponse &)> callback);

protected:
  NatsManager &hub_;

  // Subject constants
  static constexpr const char *STATE_REQUEST_SUBJECT =
      "INSTRUMENTHUB.STATE_REQUEST";
  static constexpr const char *STATE_RESPONSE_SUBJECT =
      "INSTRUMENTHUB.STATE_RESPONSE";
};

} // namespace falcon::comms
