#pragma once

#include <memory>
#include <string>

// Forward declare spdlog logger
namespace spdlog {
class logger;
}

namespace falcon::routine {

/**
 * @brief Logging utilities using spdlog
 *
 * Lazy-initialized logger. Reads configuration from environment:
 *   LOG_LEVEL - debug, info, warn, error (default: info)
 *   LOG_FILE  - path to log file (default: logs to stdout)
 *   LOG_PATTERN - custom log pattern (optional)
 *
 * Example:
 *   export LOG_LEVEL=debug
 *   export LOG_FILE=/var/log/falcon-routine.log
 */
namespace log {

/**
 * @brief Log at trace level (very verbose)
 */
void trace(const std::string &msg);

/**
 * @brief Log at debug level
 */
void debug(const std::string &msg);

/**
 * @brief Log at info level
 */
void info(const std::string &msg);

/**
 * @brief Log at warning level
 */
void warn(const std::string &msg);

/**
 * @brief Log at error level
 */
void error(const std::string &msg);

/**
 * @brief Log at critical level
 */
void critical(const std::string &msg);

/**
 * @brief Set log level programmatically
 * @param level One of: trace, debug, info, warn, error, critical, off
 */
void set_level(const std::string &level);

/**
 * @brief Flush logs to file/output
 */
void flush();

/**
 * @brief Get the underlying spdlog logger (advanced use)
 */
std::shared_ptr<spdlog::logger> get_logger();

} // namespace log

} // namespace falcon::routine
