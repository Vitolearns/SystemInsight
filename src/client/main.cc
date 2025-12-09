#include <iostream>
#include <string>

#include <gflags/gflags.h>

#include "src/client/client_app.h"
#include "src/common/config/config_loader.h"
#include "src/common/logging/logging.h"

DEFINE_string(config, "configs/client_example.json", "Path to the client JSON config file.");

int main(int argc, char** argv) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  auto config_path = FLAGS_config;
  auto client_config = system_insight::common::config::LoadClientConfig(config_path);

  system_insight::common::logging::LoggingOptions log_options;
  log_options.level = client_config.log_level;
  system_insight::common::logging::InitLogging(log_options);
  LOGI("Loaded client config from {}", config_path);

  system_insight::client::ClientApp app(client_config);
  int rc = app.Run();
  gflags::ShutDownCommandLineFlags();
  return rc;
}

