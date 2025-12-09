#include "src/client/metrics_client.h"

#include "src/common/logging/logging.h"
namespace system_insight {
namespace client {

MetricsClient::MetricsClient(std::shared_ptr<grpc::Channel> channel)
    : stub_(systeminsight::proto::SystemInsightService::NewStub(channel)) {}

bool MetricsClient::SendReport(const std::string& host_id, const std::string& collector_version,
                               const std::vector<systeminsight::proto::MetricSample>& samples) {
  systeminsight::proto::MetricsReport report;
  report.set_host_id(host_id);
  report.set_collector_version(collector_version);
  for (const auto& sample : samples) {
    *report.add_samples() = sample;
  }

  grpc::ClientContext context;
  systeminsight::proto::ReportAck ack;
  auto status = stub_->SendMetrics(&context, report, &ack);
  if (!status.ok()) {
    LOGE("SendMetrics failed: {}", status.error_message());
    return false;
  }
  LOGI("server ack: {}", ack.message());
  return ack.ok();
}

}  // namespace client
}  // namespace system_insight

