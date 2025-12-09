#ifndef SYSTEM_INSIGHT_CLIENT_METRICS_CLIENT_H_
#define SYSTEM_INSIGHT_CLIENT_METRICS_CLIENT_H_

#include <memory>
#include <string>
#include <vector>

#include "grpcpp/grpcpp.h"
#include "system_insight.grpc.pb.h"

namespace system_insight {
namespace client {

class MetricsClient {
 public:
  explicit MetricsClient(std::shared_ptr<grpc::Channel> channel);

  bool SendReport(const std::string& host_id, const std::string& collector_version,
                  const std::vector<systeminsight::proto::MetricSample>& samples);

 private:
  std::unique_ptr<systeminsight::proto::SystemInsightService::Stub> stub_;
};

}  // namespace client
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_CLIENT_METRICS_CLIENT_H_

