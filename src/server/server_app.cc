#include "src/server/server_app.h"

#include <chrono>
#include <stdexcept>
#include <utility>

#include "grpcpp/server_builder.h"
#include "src/common/logging/logging.h"

namespace system_insight {
namespace server {

ServerApp::ServerApp(common::config::ServerConfig config,
                     std::shared_ptr<MetricsRepository> repository): 
      config_(std::move(config)),
      repository_(repository ? std::move(repository) : std::make_shared<MetricsRepository>()),
      exporter_(std::make_unique<exporter::PrometheusExporter>(repository_, config_.prometheus_http_port)),
      service_(repository_) {}

ServerApp::~ServerApp() { Shutdown(); }

void ServerApp::SetListenAddress(std::string address) {
  config_.listen_address = std::move(address);
}

void ServerApp::Run() {
  grpc::ServerBuilder builder;
  builder.AddListeningPort(config_.listen_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service_);
  server_ = builder.BuildAndStart();

  if (!server_) {
    throw std::runtime_error("Failed to start gRPC server");
  }

  LOGI("server listening on {}", config_.listen_address);
  exporter_->Start();

  server_->Wait();
}

void ServerApp::RequestStop() {
  // 使用立即关闭来快速响应信号
  if (server_) {
    server_->Shutdown(std::chrono::system_clock::now());
  }
}

// 优雅关闭，在析构里执行
void ServerApp::Shutdown() {
  if (server_) {
    server_->Shutdown();
    server_.reset();
  }
  if (exporter_) {
    exporter_->Stop();
  }
}

}  // namespace server
}  // namespace system_insight

