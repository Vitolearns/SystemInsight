#ifndef SYSTEM_INSIGHT_CLIENT_SYSTEM_METRICS_COLLECTOR_H_
#define SYSTEM_INSIGHT_CLIENT_SYSTEM_METRICS_COLLECTOR_H_

#include <chrono>
#include <string>
#include <vector>

#include "system_insight.pb.h"

namespace system_insight {
namespace client {

class SystemMetricsCollector {
 public:
  SystemMetricsCollector();

  std::vector<systeminsight::proto::MetricSample> Collect();

 private:
  bool ReadCpuTimes(uint64_t* idle, uint64_t* total) const;
  bool ReadMemInfo(uint64_t* total, uint64_t* available) const;
  bool ReadNetDev(uint64_t* rx_bytes, uint64_t* tx_bytes) const;

  std::chrono::steady_clock::time_point previous_sample_time_;
  uint64_t prev_cpu_idle_ = 0;
  uint64_t prev_cpu_total_ = 0;
  uint64_t prev_rx_bytes_ = 0;
  uint64_t prev_tx_bytes_ = 0;
  bool has_cpu_baseline_ = false;
  bool has_net_baseline_ = false;
};

}  // namespace client
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_CLIENT_SYSTEM_METRICS_COLLECTOR_H_

