#include "src/client/client_app.h"

#include <chrono>
#include <thread>

#include "src/common/logging/logging.h"

namespace system_insight {
namespace client {

ClientApp::ClientApp(common::config::ClientConfig config) : config_(std::move(config)) {}

void ClientApp::RequestStop() {
  should_exit_.store(true);
}

int ClientApp::Run() {
  auto channel = grpc::CreateChannel(config_.target, grpc::InsecureChannelCredentials());
  MetricsClient client(channel);
  
  // 构建采集器配置
  CollectorConfig collector_config;
  collector_config.use_mmap = config_.use_mmap;
  collector_config.mmap_cpu_device_path = config_.mmap_cpu_device_path;
  collector_config.mmap_softirq_device_path = config_.mmap_softirq_device_path;
  
  SystemMetricsCollector collector(collector_config);
  
  LOGI("Client loop started: target={}, interval_ms={}, use_mmap={}", 
       config_.target, config_.collection_interval_ms, config_.use_mmap);

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
