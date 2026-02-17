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
  std::atomic<bool> done{false};

  std::string subject = make_measure_response_subject();

  hub_.subscribe(subject, [&prom, &done](const std::string &data) {
    if (done.exchange(true)) {
      return;
    }
    try {
      auto json = nlohmann::json::parse(data);
      MeasureResponse response = MeasureResponse::from_json(json);
      prom.set_value(response);
    } catch (const std::exception &e) {
      try {
        prom.set_exception(std::make_exception_ptr(e));
      } catch (const std::future_error &) {
      }
    }
  });

  MeasureCommand req;
  req.timestamp = time;
  hub_.publish(make_measure_command_subject(), req.to_json().dump());

  try {
    if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
        std::future_status::ready) {
      auto result = fut.get();
      hub_.unsubscribe(subject); // or subscription_id
      return result;
    }

    done = true;
    hub_.unsubscribe(subject);
    throw std::runtime_error("Timeout waiting for MeasureResponse");
  } catch (...) {
    hub_.unsubscribe(subject);
    throw;
  }
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
