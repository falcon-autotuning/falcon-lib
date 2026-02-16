#include "falcon-comms/autotuner_comms.hpp"
#include <chrono>
#include <spdlog/spdlog.h>

namespace falcon::comms {

AutotunerComms::AutotunerComms() : hub_(NatsManager::instance()) {}

std::optional<StateResponse> AutotunerComms::request_state(int timeout_ms) {
  // Create StateRequest
  StateRequest req;
  req.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                      std::chrono::system_clock::now().time_since_epoch())
                      .count();

  // Publish request and wait for response
  auto response =
      hub_.request_json(STATE_REQUEST_SUBJECT, req.to_json(), timeout_ms);

  if (!response) {
    spdlog::warn("State request timed out");
    return std::nullopt;
  }

  try {
    return StateResponse::from_json(*response);
  } catch (const std::exception &e) {
    spdlog::error("Failed to parse StateResponse: {}", e.what());
    return std::nullopt;
  }
}

void AutotunerComms::subscribe_state_response(
    std::function<void(const StateResponse &)> callback) {
  hub_.subscribe(STATE_RESPONSE_SUBJECT, [callback](const std::string &data) {
    try {
      auto json = nlohmann::json::parse(data);
      StateResponse response = StateResponse::from_json(json);
      callback(response);
    } catch (const std::exception &e) {
      spdlog::error("Failed to parse StateResponse in subscription: {}",
                    e.what());
    }
  });
}

} // namespace falcon::comms
