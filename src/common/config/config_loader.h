#ifndef SYSTEM_INSIGHT_COMMON_CONFIG_CONFIG_LOADER_H_
#define SYSTEM_INSIGHT_COMMON_CONFIG_CONFIG_LOADER_H_

#include <string>

namespace system_insight {
namespace common {
namespace config {

struct ClientConfig {
  std::string target = "127.0.0.1:50052";
  int collection_interval_ms = 5000;
  std::string log_level = "info";
  std::string host_id;
  
  // mmap 采集器配置
  bool use_mmap = false;
  std::string mmap_cpu_device_path = "/dev/system_insight_cpu_stat";
  std::string mmap_softirq_device_path = "/dev/system_insight_softirq";
};

struct ServerConfig {
  std::string listen_address = "0.0.0.0:50052";
  std::string log_level = "info";
  int prometheus_http_port = 9102;
};

ClientConfig LoadClientConfig(const std::string& path);
ServerConfig LoadServerConfig(const std::string& path);

}  // namespace config
}  // namespace common
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_COMMON_CONFIG_CONFIG_LOADER_H_
