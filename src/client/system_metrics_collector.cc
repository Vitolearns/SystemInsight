#include "src/client/system_metrics_collector.h"

#include <cstdint>
#include <fstream>
#include <numeric>
#include <sstream>

#include "src/common/logging/logging.h"

namespace system_insight {
namespace client {

SystemMetricsCollector::SystemMetricsCollector()
    : previous_sample_time_(std::chrono::steady_clock::now()) {}

std::vector<systeminsight::proto::MetricSample> SystemMetricsCollector::Collect() {
  std::vector<systeminsight::proto::MetricSample> samples;

  uint64_t idle = 0;
  uint64_t total = 0;
  if (ReadCpuTimes(&idle, &total)) {
    if (has_cpu_baseline_) {
      const double total_delta = static_cast<double>(total - prev_cpu_total_);
      const double idle_delta = static_cast<double>(idle - prev_cpu_idle_);
      if (total_delta > 0) {
        double usage = (total_delta - idle_delta) / total_delta * 100.0;
        auto& sample = samples.emplace_back();
        sample.set_name("system.cpu.usage_percent");
        sample.set_value(usage);
        sample.set_timestamp_ms(std::chrono::duration_cast<std::chrono::milliseconds>(
                                    std::chrono::system_clock::now().time_since_epoch())
                                    .count());
      }
    }
    prev_cpu_idle_ = idle;
    prev_cpu_total_ = total;
    has_cpu_baseline_ = true;
  }

  uint64_t mem_total = 0;
  uint64_t mem_available = 0;
  if (ReadMemInfo(&mem_total, &mem_available) && mem_total > 0) {
    double used_ratio = static_cast<double>(mem_total - mem_available) / mem_total * 100.0;
    auto& mem_usage = samples.emplace_back();
    mem_usage.set_name("system.mem.usage_percent");
    mem_usage.set_value(used_ratio);
    mem_usage.set_timestamp_ms(std::chrono::duration_cast<std::chrono::milliseconds>(
                                   std::chrono::system_clock::now().time_since_epoch())
                                   .count());

    auto& mem_available_sample = samples.emplace_back();
    mem_available_sample.set_name("system.mem.available_bytes");
    mem_available_sample.set_value(static_cast<double>(mem_available) * 1024.0);
    mem_available_sample.set_timestamp_ms(mem_usage.timestamp_ms());
  }

  uint64_t rx_bytes = 0;
  uint64_t tx_bytes = 0;
  if (ReadNetDev(&rx_bytes, &tx_bytes)) {
    auto now = std::chrono::steady_clock::now();
    double elapsed_sec = std::chrono::duration<double>(now - previous_sample_time_).count();
    if (elapsed_sec > 0 && has_net_baseline_) {
      double rx_rate = static_cast<double>(rx_bytes - prev_rx_bytes_) / elapsed_sec;
      double tx_rate = static_cast<double>(tx_bytes - prev_tx_bytes_) / elapsed_sec;

      auto& rx_sample = samples.emplace_back();
      rx_sample.set_name("system.net.rx_bytes_per_sec");
      rx_sample.set_value(rx_rate);
      rx_sample.set_timestamp_ms(std::chrono::duration_cast<std::chrono::milliseconds>(
                                     std::chrono::system_clock::now().time_since_epoch())
                                     .count());

      auto& tx_sample = samples.emplace_back();
      tx_sample.set_name("system.net.tx_bytes_per_sec");
      tx_sample.set_value(tx_rate);
      tx_sample.set_timestamp_ms(rx_sample.timestamp_ms());
    }
    prev_rx_bytes_ = rx_bytes;
    prev_tx_bytes_ = tx_bytes;
    previous_sample_time_ = now;
    has_net_baseline_ = true;
  }

  return samples;
}

bool SystemMetricsCollector::ReadCpuTimes(uint64_t* idle, uint64_t* total) const {
  std::ifstream file("/proc/stat");
  if (!file.is_open()) {
    LOGW("Failed to open /proc/stat");
    return false;
  }
  std::string line;
  if (!std::getline(file, line)) {
    return false;
  }
  std::istringstream ss(line);
  std::string cpu_label;
  ss >> cpu_label;
  if (cpu_label.rfind("cpu", 0) != 0) {
    return false;
  }
  uint64_t user = 0, nice = 0, system = 0, idle_time = 0, iowait = 0;
  uint64_t irq = 0, softirq = 0, steal = 0;
  ss >> user >> nice >> system >> idle_time >> iowait >> irq >> softirq >> steal;
  *idle = idle_time + iowait;
  *total = user + nice + system + idle_time + iowait + irq + softirq + steal;
  return true;
}

bool SystemMetricsCollector::ReadMemInfo(uint64_t* total, uint64_t* available) const {
  std::ifstream file("/proc/meminfo");
  if (!file.is_open()) {
    LOGW("Failed to open /proc/meminfo");
    return false;
  }
  std::string line;
  while (std::getline(file, line)) {
    if (line.rfind("MemTotal", 0) == 0) {
      std::istringstream ss(line);
      std::string key, unit;
      ss >> key >> *total >> unit;
    }
    if (line.rfind("MemAvailable", 0) == 0) {
      std::istringstream ss(line);
      std::string key, unit;
      ss >> key >> *available >> unit;
    }
  }
  return *total > 0 && *available > 0;
}

bool SystemMetricsCollector::ReadNetDev(uint64_t* rx_bytes, uint64_t* tx_bytes) const {
  std::ifstream file("/proc/net/dev");
  if (!file.is_open()) {
    LOGW("Failed to open /proc/net/dev");
    return false;
  }
  std::string line;
  // Skip headers
  std::getline(file, line);
  std::getline(file, line);
  while (std::getline(file, line)) {
    auto colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
      continue;
    }
    std::string iface = line.substr(0, colon_pos);
    iface.erase(0, iface.find_first_not_of(' '));
    if (iface == "lo") {
      continue;
    }
    std::istringstream ss(line.substr(colon_pos + 1));
    uint64_t iface_rx = 0;
    uint64_t iface_tx = 0;
    ss >> iface_rx;
    // Skip fields until tx bytes (8th after rx). We'll read total of 8 numbers then tx.
    uint64_t discard = 0;
    for (int i = 0; i < 7; ++i) {
      ss >> discard;
    }
    ss >> iface_tx;
    *rx_bytes += iface_rx;
    *tx_bytes += iface_tx;
  }
  return true;
}

}  // namespace client
}  // namespace system_insight

