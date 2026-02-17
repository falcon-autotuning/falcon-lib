#include "falcon-comms/routine_comms.hpp"
#include "falcon-comms/commands_definitions.hpp"
#include <future>
#include <spdlog/spdlog.h>

namespace {
std::string make_measure_command_subject() {
  return "INSTRUMENTHUB." + std::string(MeasureCommand::NAME);
}
std::string make_measure_response_subject() {
  return "FALCON." + std::string(MeasureResponse::NAME);
}
} // namespace
namespace falcon::comms {

RoutineComms::RoutineComms() = default;
MeasureResponse RoutineComms::subscribe_measure_response(int timeout_ms,
                                                         int time) {
  std::promise<MeasureResponse> prom;
  auto fut = prom.get_future();

  // Subscribe with a one-shot callback
  hub_.subscribe(
      make_measure_response_subject(), [&prom](const std::string &data) {
        try {
          auto json = nlohmann::json::parse(data);
          MeasureResponse response = MeasureResponse::from_json(json);
          prom.set_value(response);
        } catch (const std::exception &e) {
          spdlog::error("Failed to parse MeasureResponse in subscription: {}",
                        e.what());
        }
      });

  StateRequest req;
  req.timestamp = time;

  hub_.publish(make_measure_command_subject(), req.to_json());
  // Wait for the response or timeout
  if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
      std::future_status::ready) {
    // Optionally unsubscribe here if your hub supports it, using sub_id
    return fut.get();
  } // Optionally unsubscribe here if your hub supports it, using sub_id
  throw std::runtime_error("Timeout waiting for MeasureResponse");
}

std::vector<std::string> RoutineComms::pull_measurement_data(
    const std::string &stream, const std::string &channel, int batch_size) {
  try {
    return hub_.jetstream_pull(stream, channel, batch_size);
  } catch (const std::exception &e) {
    spdlog::error("Failed to pull measurement data from JetStream: {}",
                  e.what());
    return {};
  }
}

} // namespace falcon::comms
