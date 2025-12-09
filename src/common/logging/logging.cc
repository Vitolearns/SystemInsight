#include "src/common/logging/logging.h"

#include <mutex>
#include <vector>

#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace system_insight {
namespace common {
namespace logging {

namespace {
std::shared_ptr<spdlog::logger> g_logger;
std::mutex g_logger_mutex;

spdlog::level::level_enum ParseLevel(std::string level) {
  try {
    return spdlog::level::from_str(level);
  } catch (const spdlog::spdlog_ex&) {
    return spdlog::level::info;
  }
}

std::shared_ptr<spdlog::logger> CreateLogger(const LoggingOptions& options) {
  std::vector<spdlog::sink_ptr> sinks;
  if (options.enable_console) {
    sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_mt>());
  }
  if (options.enable_file && !options.file_path.empty()) {
    sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_mt>(options.file_path, true));
  }
  if (sinks.empty()) {
    sinks.push_back(std::make_shared<spdlog::sinks::stderr_color_sink_mt>());
  }

  auto logger = std::make_shared<spdlog::logger>(options.logger_name, sinks.begin(), sinks.end());
  logger->set_pattern(options.pattern);
  logger->set_level(ParseLevel(options.level));
  return logger;
}
}  // namespace

void InitLogging(const LoggingOptions& options) {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  g_logger = CreateLogger(options);
  spdlog::register_logger(g_logger);
  spdlog::set_default_logger(g_logger);
}

void ShutdownLogging() {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  g_logger.reset();
  spdlog::shutdown();
}

std::shared_ptr<spdlog::logger> GetLogger() {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  if (!g_logger) {
    g_logger = CreateLogger(LoggingOptions{});
    spdlog::register_logger(g_logger);
    spdlog::set_default_logger(g_logger);
  }
  return g_logger;
}

void SetLogLevel(const std::string& level) {
  std::lock_guard<std::mutex> lock(g_logger_mutex);
  if (!g_logger) {
    g_logger = CreateLogger(LoggingOptions{});
    spdlog::register_logger(g_logger);
    spdlog::set_default_logger(g_logger);
  }
  g_logger->set_level(ParseLevel(level));
}

void TriggerFatalAbort() {
  spdlog::shutdown();
  std::abort();
}

}  // namespace logging
}  // namespace common
}  // namespace system_insight

