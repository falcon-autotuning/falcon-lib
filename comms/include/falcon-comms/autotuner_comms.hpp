#pragma once

#include "commands_definitions.hpp"
#include "natsManager.hpp"
#include <functional>
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
   * @brief Subscribe to state responses with a callback
   * @param timeout_ms the timeout in milliseconds to wait
   * @param time current time
   * @return StateResponse if successful
   */
  StateResponse subscribe_state_response(int timeout_ms, int time);

protected:
  NatsManager &hub_;
};

} // namespace falcon::comms
