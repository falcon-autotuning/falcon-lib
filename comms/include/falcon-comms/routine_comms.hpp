#pragma once

#include "autotuner_comms.hpp"
#include "commands_definitions.hpp"
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
   * @brief Subscribe to measure responses with a callback
   * @param timeout_ms the timeout in milliseconds to wait
   * @param time current time
   * @return MeasureResponse if successful
   */
  MeasureResponse subscribe_measure_response(int timeout_ms, int time);

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

private:
};

} // namespace falcon::comms
