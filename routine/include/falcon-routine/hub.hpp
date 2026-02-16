#pragma once

#include <functional>
#include <memory>
#include <mutex>
#include <nats/nats.h>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>

namespace falcon::routine {

/**
 * @brief Lazy-initialized NATS hub for messaging
 *
 * Connects to NATS on first use. Thread-safe.
 * Reads NATS_URL from environment.
 *
 * Environment: NATS_URL (e.g., "nats://nats:4222")
 */
class Hub {
public:
  /**
   * @brief Get the singleton instance
   */
  static Hub &instance();

  /**
   * @brief Publish a message to a subject
   * @param subject NATS subject (e.g., "measurements.voltage")
   * @param message Message payload
   */
  void publish(const std::string &subject, const std::string &message);

  /**
   * @brief Publish JSON data to a subject
   * @param subject NATS subject
   * @param data JSON data to publish
   */
  void publish_json(const std::string &subject, const nlohmann::json &data);

  /**
   * @brief Send a request and wait for response
   * @param subject NATS subject
   * @param request Request payload
   * @param timeout_ms Timeout in milliseconds (default: 5000)
   * @return Response if received, nullopt if timeout
   */
  std::optional<std::string> request(const std::string &subject,
                                     const std::string &request,
                                     int timeout_ms = 5000);

  /**
   * @brief Send a request with JSON and receive JSON response
   * @param subject NATS subject
   * @param request JSON request data
   * @param timeout_ms Timeout in milliseconds
   * @return JSON response if received, nullopt if timeout
   */
  std::optional<nlohmann::json> request_json(const std::string &subject,
                                             const nlohmann::json &request,
                                             int timeout_ms = 5000);

  /**
   * @brief Subscribe to a subject with a callback
   * @param subject NATS subject (can include wildcards)
   * @param callback Function to call when message received
   */
  void subscribe(const std::string &subject,
                 std::function<void(const std::string &)> callback);

  /**
   * @brief Check if connected to NATS
   */
  bool is_connected() const;

  /**
   * @brief Manually connect (usually not needed - auto-connects on first use)
   * @param url NATS URL (if empty, uses NATS_URL environment variable)
   */
  void connect(const std::string &url = "");

  /**
   * @brief Disconnect from NATS
   */
  void disconnect();

  ~Hub();

private:
  Hub() = default;
  Hub(const Hub &) = delete;
  Hub &operator=(const Hub &) = delete;

  void ensure_connected();

  natsConnection *conn_ = nullptr;
  mutable std::mutex mutex_;
};

} // namespace falcon::routine
