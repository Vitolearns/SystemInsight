#ifndef SYSTEM_INSIGHT_COMMON_LOGGING_LOGGING_H_
#define SYSTEM_INSIGHT_COMMON_LOGGING_LOGGING_H_

#include <memory>
#include <string>

#include "spdlog/spdlog.h"

namespace system_insight {
namespace common {
namespace logging {

struct LoggingOptions {
  std::string logger_name = "system_insight";
  std::string level = "info";
  std::string pattern = "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v";
  bool enable_console = true;
  bool enable_file = false;
  std::string file_path;
};

void InitLogging(const LoggingOptions& options = LoggingOptions());
void ShutdownLogging();
std::shared_ptr<spdlog::logger> GetLogger();
void SetLogLevel(const std::string& level);
void TriggerFatalAbort();

}  // namespace logging
}  // namespace common
}  // namespace system_insight

#define SYSTEM_INSIGHT_LOGGER (::system_insight::common::logging::GetLogger().get())
#define LOGD(fmt, ...) SPDLOG_LOGGER_CALL(SYSTEM_INSIGHT_LOGGER, spdlog::level::debug, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) SPDLOG_LOGGER_CALL(SYSTEM_INSIGHT_LOGGER, spdlog::level::info, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) SPDLOG_LOGGER_CALL(SYSTEM_INSIGHT_LOGGER, spdlog::level::warn, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) SPDLOG_LOGGER_CALL(SYSTEM_INSIGHT_LOGGER, spdlog::level::err, fmt, ##__VA_ARGS__)
#define LOGF(fmt, ...)                                                                        \
  do {                                                                                        \
    SPDLOG_LOGGER_CALL(SYSTEM_INSIGHT_LOGGER, spdlog::level::critical, fmt, ##__VA_ARGS__);   \
    ::system_insight::common::logging::TriggerFatalAbort();                                   \
  } while (0)

#endif  // SYSTEM_INSIGHT_COMMON_LOGGING_LOGGING_H_

