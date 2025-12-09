#ifndef SYSTEM_INSIGHT_SERVER_METRICS_SERVICE_IMPL_H_
#define SYSTEM_INSIGHT_SERVER_METRICS_SERVICE_IMPL_H_

#include <memory>

#include "grpcpp/grpcpp.h"
#include "system_insight.grpc.pb.h"
#include "src/server/metrics_repository.h"

namespace system_insight {
namespace server {

class MetricsServiceImpl final : public systeminsight::proto::SystemInsightService::Service {
 public:
  explicit MetricsServiceImpl(std::shared_ptr<MetricsRepository> repository);

  grpc::Status SendMetrics(grpc::ServerContext* context,
                           const systeminsight::proto::MetricsReport* request,
                           systeminsight::proto::ReportAck* response) override;

 private:
  std::shared_ptr<MetricsRepository> repository_;
};

}  // namespace server
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_SERVER_METRICS_SERVICE_IMPL_H_

