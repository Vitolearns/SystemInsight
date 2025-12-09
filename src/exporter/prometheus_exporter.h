#ifndef SYSTEM_INSIGHT_EXPORTER_PROMETHEUS_EXPORTER_H_
#define SYSTEM_INSIGHT_EXPORTER_PROMETHEUS_EXPORTER_H_

#include <atomic>
#include <memory>
#include <string>
#include <thread>

#include "src/server/metrics_repository.h"

namespace system_insight {
namespace exporter {

class PrometheusExporter {
 public:
  PrometheusExporter(std::shared_ptr<server::MetricsRepository> repository, int port);
  ~PrometheusExporter();

  void Start();
  void Stop();

 private:
  void Serve();
  std::string BuildMetricsPayload() const;
  static std::string SanitizeMetricName(const std::string& name);
  static std::string EscapeLabelValue(const std::string& value);

  std::shared_ptr<server::MetricsRepository> repository_;
  int port_;
  std::atomic<bool> running_{false};
  std::thread server_thread_;
  int server_fd_ = -1;
};

}  // namespace exporter
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_EXPORTER_PROMETHEUS_EXPORTER_H_

