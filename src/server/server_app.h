#ifndef SYSTEM_INSIGHT_SERVER_SERVER_APP_H_
#define SYSTEM_INSIGHT_SERVER_SERVER_APP_H_

#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "src/common/config/config_loader.h"
#include "src/exporter/prometheus_exporter.h"
#include "src/server/metrics_repository.h"
#include "src/server/metrics_service_impl.h"

namespace system_insight {
namespace server {

class ServerApp {
 public:
  ServerApp(common::config::ServerConfig config,
            std::shared_ptr<MetricsRepository> repository = nullptr);
  ~ServerApp();

  ServerApp(const ServerApp&) = delete;
  ServerApp& operator=(const ServerApp&) = delete;

  void SetListenAddress(std::string address);
  // 阻塞运行，直到 Shutdown 被调用或捕获终止信号。
  void Run();
  void RequestStop();
  void Shutdown();

 private:
  common::config::ServerConfig config_;
  std::shared_ptr<MetricsRepository> repository_;
  std::unique_ptr<exporter::PrometheusExporter> exporter_;
  std::unique_ptr<grpc::Server> server_;
  MetricsServiceImpl service_;
};

}  // namespace server
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_SERVER_SERVER_APP_H_

