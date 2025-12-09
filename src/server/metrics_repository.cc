#include "src/server/metrics_repository.h"

#include <utility>

namespace system_insight {
namespace server {

void MetricsRepository::UpdateReport(const systeminsight::proto::MetricsReport& report) {
  std::lock_guard<std::mutex> lock(mutex_);
  reports_[report.host_id()] = report;
}

std::vector<systeminsight::proto::MetricsReport> MetricsRepository::Snapshot() const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<systeminsight::proto::MetricsReport> snapshot;
  snapshot.reserve(reports_.size());
  for (const auto& [host, report] : reports_) {
    snapshot.push_back(report);
  }
  return snapshot;
}

}  // namespace server
}  // namespace system_insight

