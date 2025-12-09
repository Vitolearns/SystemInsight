#ifndef SYSTEM_INSIGHT_CLIENT_CLIENT_APP_H_
#define SYSTEM_INSIGHT_CLIENT_CLIENT_APP_H_

#include <atomic>
#include <memory>
#include <string>

#include "grpcpp/grpcpp.h"
#include "src/client/metrics_client.h"
#include "src/client/system_metrics_collector.h"
#include "src/common/config/config_loader.h"

namespace system_insight {
namespace client {

class ClientApp {
 public:
  explicit ClientApp(common::config::ClientConfig config);

  int Run();
  void RequestStop();

 private:
  void InstallSignalHandlers();

  common::config::ClientConfig config_;
  std::atomic<bool> should_exit_{false};
};

}  // namespace client
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_CLIENT_CLIENT_APP_H_

