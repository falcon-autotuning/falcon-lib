#include "falcon-comms/autotuner_comms.hpp"
#include <future>
#include <spdlog/spdlog.h>
namespace {
std::string make_state_request_subject() {
  return "INSTRUMENTHUB." + std::string(StateRequest::NAME);
}
std::string make_state_response_subject() {
  return "FALCON." + std::string(StateResponse::NAME);
}
} // namespace
namespace falcon::comms {

AutotunerComms::AutotunerComms() : hub_(NatsManager::instance()) {}

StateResponse AutotunerComms::subscribe_state_response(int timeout_ms,
                                                       int time) {
  std::promise<StateResponse> prom;
  auto fut = prom.get_future();

  // Subscribe with a one-shot callback
  hub_.subscribe(
      make_state_response_subject(), [&prom](const std::string &data) {
        try {
          auto json = nlohmann::json::parse(data);
          StateResponse response = StateResponse::from_json(json);
          prom.set_value(response);
        } catch (const std::exception &e) {
          spdlog::error("Failed to parse StateResponse in subscription: {}",
                        e.what());
        }
      });

  StateRequest req;
  req.timestamp = time;

  hub_.publish(make_state_request_subject(), req.to_json());
  // Wait for the response or timeout
  if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
      std::future_status::ready) {
    // Optionally unsubscribe here if your hub supports it, using sub_id
    return fut.get();
  } // Optionally unsubscribe here if your hub supports it, using sub_id
  throw std::runtime_error("Timeout waiting for StateResponse");
}

} // namespace falcon::comms
