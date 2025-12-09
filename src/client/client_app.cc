#include "src/client/client_app.h"

#include <chrono>
#include <csignal>
#include <thread>

#include "src/common/logging/logging.h"

namespace system_insight {
namespace client {

namespace {
ClientApp* g_active_app = nullptr;

void SignalHandler(int) {
  if (g_active_app != nullptr) {
    g_active_app->RequestStop();
  }
}
}  // namespace

ClientApp::ClientApp(common::config::ClientConfig config) : config_(std::move(config)) {}

void ClientApp::InstallSignalHandlers() {
  g_active_app = this;
  std::signal(SIGINT, SignalHandler);
  std::signal(SIGTERM, SignalHandler);
}

void ClientApp::RequestStop() {
  should_exit_.store(true);
}

int ClientApp::Run() {
  InstallSignalHandlers();

  auto channel = grpc::CreateChannel(config_.target, grpc::InsecureChannelCredentials());
  MetricsClient client(channel);
  SystemMetricsCollector collector;

  LOGI("Client loop started: target={}, interval_ms={}", config_.target, config_.collection_interval_ms);

  while (!should_exit_.load()) {
    auto samples = collector.Collect();
    if (!samples.empty()) {
      if (!client.SendReport(config_.host_id, "system_insight_client", samples)) {
        LOGW("Failed to send metrics batch");
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(config_.collection_interval_ms));
  }

  LOGI("Client loop exiting");
  return 0;
}

}  // namespace client
}  // namespace system_insight

