#include "falcon-routine/hub.hpp"
#include <falcon_core/communications/Time.hpp>
#include <falcon_core/communications/messages/VoltageStatesResponse.hpp>

namespace falcon::routine {
using falcon_core::communications::Time;

falcon_core::communications::messages::VoltageStatesResponseSP
request_device_state(int timeout_ms) {
  Comms comms;
  auto resp = comms.subscribe_state_response(timeout_ms, Time().time());
  return falcon_core::communications::messages::VoltageStatesResponse::
      from_json_string<
          falcon_core::communications::messages::VoltageStatesResponse>(
          resp.response);
}

falcon_core::communications::messages::MeasurementResponseSP
request_measurement(
    falcon_core::communications::messages::MeasurementRequestSP req,
    int timeout_ms) {
  Comms comms;
  std::string json_req = req->to_json_string();
  auto resp =
      comms.subscribe_measure_response(json_req, timeout_ms, Time().time());
  auto outs = comms.pull_measurement_data(resp.stream, resp.channel, 1);
  return falcon_core::communications::messages::MeasurementResponse::
      from_json_string<
          falcon_core::communications::messages::MeasurementResponse>(outs[0]);
}
} // namespace falcon::routine
