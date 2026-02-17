#include "falcon-comms/autotuner_comms.hpp"
#include <future>
#include <iostream>
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
  std::atomic<bool> done{false};

  std::string subject = make_state_response_subject();

  hub_.subscribe(subject, [&prom, &done](const std::string &data) {
    if (done.exchange(true)) {
      return;
    }
    try {
      auto json = nlohmann::json::parse(data);
      StateResponse response = StateResponse::from_json(json);
      prom.set_value(response);
    } catch (const std::exception &e) {
      try {
        prom.set_exception(std::make_exception_ptr(e));
      } catch (const std::future_error &) {
      }
    }
  });

  StateRequest req;
  req.timestamp = time;
  hub_.publish(make_state_request_subject(), req.to_json().dump());

  try {
    if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
        std::future_status::ready) {
      auto result = fut.get();
      hub_.unsubscribe(subject); // or subscription_id
      return result;
    }

    done = true;
    hub_.unsubscribe(subject);
    throw std::runtime_error("Timeout waiting for StateResponse");
  } catch (...) {
    hub_.unsubscribe(subject);
    throw;
  }
}
} // namespace falcon::comms
