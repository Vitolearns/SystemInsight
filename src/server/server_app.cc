#include "src/server/server_app.h"

#include <csignal>
#include <stdexcept>
#include <utility>

#include "grpcpp/server_builder.h"
#include "src/common/logging/logging.h"

namespace system_insight {
namespace server {

namespace {
std::function<void(int)>* g_signal_handler = nullptr;

void SignalTrampoline(int signal) {
  if (g_signal_handler != nullptr) {
    (*g_signal_handler)(signal);
  }
}
}  // namespace

ServerApp::ServerApp(common::config::ServerConfig config, std::shared_ptr<MetricsRepository> repository)
    : config_(std::move(config)),
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

  g_signal_handler = new std::function<void(int)>(
      [this](int signal) {
        LOGW("Caught signal {}, shutting down...", signal);
        Shutdown();
      });
  std::signal(SIGINT, SignalTrampoline);
  std::signal(SIGTERM, SignalTrampoline);

  server_->Wait();

  delete g_signal_handler;
  g_signal_handler = nullptr;
}

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

