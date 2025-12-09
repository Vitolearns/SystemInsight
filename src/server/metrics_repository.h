#ifndef SYSTEM_INSIGHT_SERVER_METRICS_REPOSITORY_H_
#define SYSTEM_INSIGHT_SERVER_METRICS_REPOSITORY_H_

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "system_insight.pb.h"

namespace system_insight {
namespace server {

class MetricsRepository {
 public:
  void UpdateReport(const systeminsight::proto::MetricsReport& report);
  std::vector<systeminsight::proto::MetricsReport> Snapshot() const;

 private:
  mutable std::mutex mutex_;
  std::unordered_map<std::string, systeminsight::proto::MetricsReport> reports_;
};

}  // namespace server
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_SERVER_METRICS_REPOSITORY_H_

