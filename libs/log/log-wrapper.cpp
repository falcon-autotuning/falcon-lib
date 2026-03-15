#include <cstdlib>
#include <falcon-typing/FFIHelpers.hpp>
#include <mutex>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

using namespace falcon::typing;
using namespace falcon::typing::ffi::wrapper;

namespace {
std::shared_ptr<spdlog::logger> g_logger;
std::once_flag g_init_flag;

static void pack_nil(FalconResultSlot *out, int32_t *oc) {
  out[0] = {};
  out[0].tag = FALCON_TYPE_NIL;
  *oc = 1;
}
void initialize_logger() {
  try {
    const char *log_file = std::getenv("LOG_FILE");
    const char *log_level = std::getenv("LOG_LEVEL");
    const char *log_pattern = std::getenv("LOG_PATTERN");

    // Create sinks
    std::vector<spdlog::sink_ptr> sinks;

    if (log_file && strlen(log_file) > 0) {
      // Rotating file sink: 10MB max, 3 rotated files
      auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
          log_file, 1024 * 1024 * 10, 3);
      sinks.push_back(file_sink);

      // Also add console sink for errors
      auto console_sink =
          std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
      console_sink->set_level(spdlog::level::warn); // Only errors to console
      sinks.push_back(console_sink);
    } else {
      // Console only
      auto console_sink =
          std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
      sinks.push_back(console_sink);
    }

    // Create multi-sink logger
    g_logger = std::make_shared<spdlog::logger>("falcon-dsl", sinks.begin(),
                                                sinks.end());

    // Set pattern
    if (log_pattern && strlen(log_pattern) > 0) {
      g_logger->set_pattern(log_pattern);
    } else {
      // Default pattern: [timestamp] [level] message
      g_logger->set_pattern("[%Y-%m-%d %H:%M:%S.%f] [%^%l%$] %v");
    }

    // Set level
    if (log_level && strlen(log_level) > 0) {
      std::string level_str(log_level);
      if (level_str == "trace") {
        g_logger->set_level(spdlog::level::trace);
      } else if (level_str == "debug") {
        g_logger->set_level(spdlog::level::debug);
      } else if (level_str == "info") {
        g_logger->set_level(spdlog::level::info);
      } else if (level_str == "warn" || level_str == "warning") {
        g_logger->set_level(spdlog::level::warn);
      } else if (level_str == "error") {
        g_logger->set_level(spdlog::level::err);
      } else if (level_str == "critical") {
        g_logger->set_level(spdlog::level::critical);
      } else if (level_str == "off") {
        g_logger->set_level(spdlog::level::off);
      } else {
        g_logger->set_level(spdlog::level::info); // default
      }
    } else {
      g_logger->set_level(spdlog::level::info); // default
    }

    // Flush on warning and above
    g_logger->flush_on(spdlog::level::warn);

    // Register as default logger
    spdlog::set_default_logger(g_logger);

  } catch (const spdlog::spdlog_ex &ex) {
    fprintf(stderr, "Log initialization failed: %s\n", ex.what());
    // Create fallback console logger
    g_logger = spdlog::stdout_color_mt("falcon-dsl-fallback");
  }
}

std::shared_ptr<spdlog::logger> get_logger_internal() {
  std::call_once(g_init_flag, initialize_logger);
  return g_logger;
}
} // namespace

void trace(const std::string &msg) { get_logger_internal()->trace(msg); }

void debug(const std::string &msg) { get_logger_internal()->debug(msg); }

void info(const std::string &msg) { get_logger_internal()->info(msg); }

void warn(const std::string &msg) { get_logger_internal()->warn(msg); }

void error(const std::string &msg) { get_logger_internal()->error(msg); }

void critical(const std::string &msg) { get_logger_internal()->critical(msg); }

void set_level(const std::string &level) {
  auto logger = get_logger_internal();

  if (level == "trace") {
    logger->set_level(spdlog::level::trace);
  } else if (level == "debug") {
    logger->set_level(spdlog::level::debug);
  } else if (level == "info") {
    logger->set_level(spdlog::level::info);
  } else if (level == "warn" || level == "warning") {
    logger->set_level(spdlog::level::warn);
  } else if (level == "error") {
    logger->set_level(spdlog::level::err);
  } else if (level == "critical") {
    logger->set_level(spdlog::level::critical);
  } else if (level == "off") {
    logger->set_level(spdlog::level::off);
  }
}

void flush() { get_logger_internal()->flush(); }

std::shared_ptr<spdlog::logger> get_logger() { return get_logger_internal(); }

extern "C" {

void Info(const FalconParamEntry *params, int32_t param_count,
          FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string message = std::get<std::string>(pm.at("message"));
  try {
    info(message);
    pack_nil(out, out_count);
  } catch (const std::exception &e) {
    pack_results(FunctionResult{nullptr, ErrorObject{e.what(), false}}, out, 16,
                 out_count);
  }
}

void Debug(const FalconParamEntry *params, int32_t param_count,
           FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string message = std::get<std::string>(pm.at("message"));
  try {
    debug(message);
    pack_nil(out, out_count);
  } catch (const std::exception &e) {
    pack_results(FunctionResult{nullptr, ErrorObject{e.what(), false}}, out, 16,
                 out_count);
  }
}

void Warn(const FalconParamEntry *params, int32_t param_count,
          FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string message = std::get<std::string>(pm.at("message"));
  try {
    warn(message);
    pack_nil(out, out_count);
  } catch (const std::exception &e) {
    pack_results(FunctionResult{nullptr, ErrorObject{e.what(), false}}, out, 16,
                 out_count);
  }
}

void Err(const FalconParamEntry *params, int32_t param_count,
         FalconResultSlot *out, int32_t *out_count) {
  auto pm = unpack_params(params, param_count);
  std::string message = std::get<std::string>(pm.at("message"));
  try {
    error(message);
    pack_nil(out, out_count);
  } catch (const std::exception &e) {
    pack_results(FunctionResult{nullptr, ErrorObject{e.what(), false}}, out, 16,
                 out_count);
  }
}

} // extern "C"
