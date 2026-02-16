#pragma once

#include "autotuner_comms.hpp"
#include "commands_definitions.hpp"
#include <optional>
#include <string>
#include <vector>

namespace falcon::comms {

/**
 * @brief Communication service for Routine operations
 *
 * Extends AutotunerComms to add measurement capabilities with JetStream
 * support.
 */
class RoutineComms : public AutotunerComms {
public:
  RoutineComms();
  ~RoutineComms() override = default;

  /**
   * @brief Request a measurement and pull data from JetStream
   * @param request Measurement request string
   * @param timeout_ms Timeout for response in milliseconds
   * @return Tuple of (MeasureResponse, measurement_data) if successful
   */
  std::optional<std::pair<MeasureResponse, std::vector<std::string>>>
  request_measurement(const std::string &request, int timeout_ms = 10000);

  /**
   * @brief Subscribe to measurement responses with a callback
   * @param callback Function to call when measurement response received
   */
  void subscribe_measurement_response(
      std::function<void(const MeasureResponse &)> callback);

  /**
   * @brief Publish a measurement command to instrument hub
   * @param request Measurement request string
   */
  void publish_measurement_command(const std::string &request);

private:
  // Subject constants
  static constexpr const char *MEASURE_COMMAND_SUBJECT =
      "INSTRUMENTHUB.MEASURE_COMMAND";
  static constexpr const char *MEASURE_RESPONSE_SUBJECT =
      "INSTRUMENTHUB.MEASURE_RESPONSE";

  /**
   * @brief Pull measurement data from JetStream
   * @param stream Stream name
   * @param channel Channel name (consumer)
   * @param batch_size Number of messages to pull
   * @return Vector of measurement data strings
   */
  std::vector<std::string> pull_measurement_data(const std::string &stream,
                                                 const std::string &channel,
                                                 int batch_size = 10);
};

} // namespace falcon::comms
