#include "falcon-comms/routine_comms.hpp"
#include <chrono>
#include <spdlog/spdlog.h>

namespace falcon::comms {

RoutineComms::RoutineComms() : AutotunerComms() {}

void RoutineComms::publish_measurement_command(const std::string &request) {
  MeasureCommand cmd;
  cmd.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
  cmd.request = request;

  hub_.publish_json(MEASURE_COMMAND_SUBJECT, cmd.to_json());
  spdlog::debug("Published MeasureCommand: {}", request);
}

std::optional<std::pair<MeasureResponse, std::vector<std::string>>>
RoutineComms::request_measurement(const std::string &request, int timeout_ms) {
  // Create MeasureCommand
  MeasureCommand cmd;
  cmd.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();
  cmd.request = request;

  // Publish command and wait for response
  auto response =
      hub_.request_json(MEASURE_COMMAND_SUBJECT, cmd.to_json(), timeout_ms);

  if (!response) {
    spdlog::warn("Measurement request timed out");
    return std::nullopt;
  }

  try {
    MeasureResponse measure_response = MeasureResponse::from_json(*response);

    spdlog::debug("Received MeasureResponse - stream: {}, channel: {}",
                  measure_response.stream, measure_response.channel);

    // Pull measurement data from JetStream
    auto measurement_data = pull_measurement_data(measure_response.stream,
                                                  measure_response.channel);

    return std::make_pair(measure_response, measurement_data);
  } catch (const std::exception &e) {
    spdlog::error("Failed to process measurement response: {}", e.what());
    return std::nullopt;
  }
}

void RoutineComms::subscribe_measurement_response(
    std::function<void(const MeasureResponse &)> callback) {
  hub_.subscribe(MEASURE_RESPONSE_SUBJECT, [callback](const std::string &data) {
    try {
      auto json = nlohmann::json::parse(data);
      MeasureResponse response = MeasureResponse::from_json(json);
      callback(response);
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse MeasureResponse in subscription: {}",
                    e.what());
    }
  });
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
