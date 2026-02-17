#include "falcon-comms/runtime_comms.hpp"
#include "falcon-comms/commands_definitions.hpp"
#include <chrono>
#include <future>
#include <spdlog/spdlog.h>

namespace {
std::string make_port_request_subject() {
  return "INSTRUMENTHUB." + std::string(PortRequest::NAME);
}
std::string make_port_response_subject() {
  return "FALCON." + std::string(PortPayload::NAME);
}
std::string make_config_request_subject() {
  return "INSTRUMENTHUB." + std::string(DeviceConfigRequest::NAME);
}
std::string make_config_response_subject() {
  return "FALCON." + std::string(DeviceConfigResponse::NAME);
}
} // namespace
namespace falcon::comms {

RuntimeComms::RuntimeComms() : hub_(NatsManager::instance()) {}

DeviceConfigResponse RuntimeComms::subscribe_config_response(int timeout_ms,
                                                             int time) {
  std::promise<DeviceConfigResponse> prom;
  auto fut = prom.get_future();
  std::atomic<bool> done{false};

  std::string subject = make_config_response_subject();

  hub_.subscribe(subject, [&prom, &done](const std::string &data) {
    if (done.exchange(true)) {
      return;
    }
    try {
      auto json = nlohmann::json::parse(data);
      DeviceConfigResponse response = DeviceConfigResponse::from_json(json);
      prom.set_value(response);
    } catch (const std::exception &e) {
      try {
        prom.set_exception(std::make_exception_ptr(e));
      } catch (const std::future_error &) {
      }
    }
  });

  DeviceConfigRequest req;
  req.timestamp = time;

  hub_.publish(make_config_request_subject(), req.to_json().dump());
  try {
    if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
        std::future_status::ready) {
      auto result = fut.get();
      hub_.unsubscribe(subject); // or subscription_id
      return result;
    }

    done = true;
    hub_.unsubscribe(subject);
    throw std::runtime_error("Timeout waiting for DeviceConfigResponse");
  } catch (...) {
    hub_.unsubscribe(subject);
    throw;
  }
}

PortPayload RuntimeComms::subscribe_port_payload(int timeout_ms, int time) {
  std::promise<PortPayload> prom;
  auto fut = prom.get_future();
  std::atomic<bool> done{false};

  std::string subject = make_port_response_subject();

  hub_.subscribe(subject, [&prom, &done](const std::string &data) {
    if (done.exchange(true)) {
      return;
    }
    try {
      auto json = nlohmann::json::parse(data);
      PortPayload response = PortPayload::from_json(json);
      prom.set_value(response);
    } catch (const std::exception &e) {
      try {
        prom.set_exception(std::make_exception_ptr(e));
      } catch (const std::future_error &) {
      }
    }
  });

  PortRequest req;
  req.timestamp = time;
  hub_.publish(make_port_request_subject(), req.to_json().dump());

  try {
    if (fut.wait_for(std::chrono::milliseconds(timeout_ms)) ==
        std::future_status::ready) {
      auto result = fut.get();
      hub_.unsubscribe(subject); // or subscription_id
      return result;
    }

    done = true;
    hub_.unsubscribe(subject);
    throw std::runtime_error("Timeout waiting for PortResponse");
  } catch (...) {
    hub_.unsubscribe(subject);
    throw;
  }
}
} // namespace falcon::comms
