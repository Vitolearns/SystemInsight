#include <csignal>
#include <iostream>

#include <gflags/gflags.h>

#include "src/common/config/config_loader.h"
#include "src/common/logging/logging.h"
#include "src/server/server_app.h"

DEFINE_string(config, "configs/server_example.json", "Path to the server JSON config file.");

namespace {
system_insight::server::ServerApp* g_active_app = nullptr;

void SignalHandler(int signal) {
  if (g_active_app != nullptr) {
    g_active_app->RequestStop();
  }
}
}  // namespace

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  auto config_path = FLAGS_config;
  auto server_config = system_insight::common::config::LoadServerConfig(config_path);

  system_insight::common::logging::LoggingOptions log_options;
  log_options.level = server_config.log_level;
  system_insight::common::logging::InitLogging(log_options);
  LOGI("Loaded server config from {}", config_path);

  try {
    system_insight::server::ServerApp app(server_config);
    g_active_app = &app;
    std::signal(SIGINT, SignalHandler);
    std::signal(SIGTERM, SignalHandler);

    LOGI("system_insight server starting on {}", server_config.listen_address);
    app.Run();
  } catch (const std::exception& ex) {
    LOGF("Fatal exception: {}", ex.what());
    return 1;
  }

  gflags::ShutDownCommandLineFlags();
  return 0;
}

