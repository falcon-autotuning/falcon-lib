#pragma once
#include "falcon-comms/routine_comms.hpp"
#include "falcon_core/communications/messages/MeasurementRequest.hpp"

namespace falcon::routine {

/**
 * @brief Alias for Routine Comms but exposed within the routine
 * library.
 */
using Comms = falcon::comms::RoutineComms;

std::optional<StateResponse>
AutotunerComms::request_state_async(int timeout_ms) {
  std::promise<StateResponse> prom;
  auto fut = prom.get_future();
  subscribe_state_response(
      [&prom](const StateResponse &resp) { prom.set_value(resp); });
  // Send request
  request_state();
  if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
      std::future_status::ready) {
    return fut.get();
  } else {
    return std::nullopt;
  }

} // namespace falcon::routine
