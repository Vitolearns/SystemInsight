#include "src/server/metrics_service_impl.h"

#include "src/common/logging/logging.h"

namespace system_insight {
namespace server {

MetricsServiceImpl::MetricsServiceImpl(std::shared_ptr<MetricsRepository> repository)
    : repository_(std::move(repository)) {}

grpc::Status MetricsServiceImpl::SendMetrics(
    grpc::ServerContext* /*context*/, 
    const systeminsight::proto::MetricsReport* request,
    systeminsight::proto::ReportAck* response) {
  LOGI("received {} samples from host {}", request->samples_size(), request->host_id());
  if (repository_) {
    repository_->UpdateReport(*request);
  }
  response->set_ok(true);
  response->set_message("accepted");
  return grpc::Status::OK;
}

}  // namespace server
}  // namespace system_insight

