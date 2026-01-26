#include "src/client/metrics/cpu_mmap_collector.h"

#include <algorithm>
#include <cmath>
#include <numeric>

#include "src/common/logging/logging.h"

namespace system_insight {
namespace client {

// Forward declaration
int64_t GetCurrentTimestampMs();

CpuMmapCollector::CpuMmapCollector(const std::string& device_path,
                                   const std::string& softirq_device_path)
    : cpu_reader_(device_path, MAX_CPUS, sizeof(CpuStatData)),
      softirq_reader_(softirq_device_path, MAX_CPUS, sizeof(SoftirqStatData)),
      softirq_device_path_(softirq_device_path),
      previous_sample_time_(std::chrono::steady_clock::now()) {}

std::vector<systeminsight::proto::MetricSample> CpuMmapCollector::Collect() {
  std::vector<systeminsight::proto::MetricSample> samples;

  if (!cpu_reader_.IsValid()) {
    LOGW("CPU mmap reader not available: %s", cpu_reader_.GetLastError().c_str());
  }

  if (!softirq_reader_.IsValid() && !softirq_device_path_.empty()) {
    LOGW("Softirq mmap reader not available: %s", softirq_reader_.GetLastError().c_str());
  }

  // 采集各类指标
  CollectCpuUsage(samples);
  CollectPerCpuCore(samples);
  CollectSoftirq(samples);

  previous_sample_time_ = std::chrono::steady_clock::now();
  has_baseline_ = true;

  return samples;
}

void CpuMmapCollector::CollectCpuUsage(std::vector<systeminsight::proto::MetricSample>& samples) {
  if (!cpu_reader_.IsValid()) return;

  auto* data = static_cast<CpuStatData*>(cpu_reader_.GetData());
  int count = cpu_reader_.GetValidCount();

  uint64_t total_idle = 0;
  uint64_t total_iowait = 0;
  uint64_t total_all = 0;
  uint64_t prev_total_idle = 0;
  uint64_t prev_total_iowait = 0;
  uint64_t prev_total_all = 0;

  // 汇总所有 CPU 的数据
  for (int i = 0; i < count; ++i) {
    const auto& stat = data[i];
    if (stat.cpu_name[0] == '\0') break;

    // 跳过聚合行 "cpu"（如果有的话）
    if (stat.cpu_name[3] == '\0') continue;

    total_idle += stat.idle;
    total_iowait += stat.iowait;
    total_all += stat.user + stat.nice + stat.system + stat.idle +
                 stat.iowait + stat.irq + stat.softirq + stat.steal;

    // 累加历史数据
    auto it = prev_cpu_stats_.find(stat.cpu_name);
    if (it != prev_cpu_stats_.end()) {
      prev_total_idle += it->second.idle;
      prev_total_iowait += it->second.iowait;
      prev_total_all += it->second.user + it->second.nice + it->second.system +
                       it->second.idle + it->second.iowait + it->second.irq +
                       it->second.softirq + it->second.steal;
    }
  }

  // 计算整体 CPU 使用率
  if (has_baseline_ && prev_total_all > 0) {
    double usage = CalculateCpuUsage(total_idle, total_iowait, total_all,
                                     prev_total_idle, prev_total_iowait, prev_total_all);
    
    auto& sample = samples.emplace_back();
    sample.set_name("system.cpu.usage_percent");
    sample.set_value(usage);
    sample.set_timestamp_ms(GetCurrentTimestampMs());
  }

  // 保存当前数据作为下一次的历史
  prev_cpu_stats_.clear();
  for (int i = 0; i < count; ++i) {
    if (data[i].cpu_name[0] != '\0') {
      prev_cpu_stats_[data[i].cpu_name] = data[i];
    }
  }
}

void CpuMmapCollector::CollectPerCpuCore(std::vector<systeminsight::proto::MetricSample>& samples) {
  if (!cpu_reader_.IsValid()) return;

  auto* data = static_cast<CpuStatData*>(cpu_reader_.GetData());
  int count = cpu_reader_.GetValidCount();

  for (int i = 0; i < count; ++i) {
    const auto& stat = data[i];
    if (stat.cpu_name[0] == '\0') break;

    // 跳过聚合行 "cpu"
    if (stat.cpu_name[3] == '\0') continue;

    auto it = prev_cpu_stats_.find(stat.cpu_name);
    if (it == prev_cpu_stats_.end() || !has_baseline_) continue;

    const auto& prev = it->second;

    uint64_t total = stat.user + stat.nice + stat.system + stat.idle +
                     stat.iowait + stat.irq + stat.softirq + stat.steal;
    uint64_t prev_total = prev.user + prev.nice + prev.system + prev.idle +
                          prev.iowait + prev.irq + prev.softirq + prev.steal;
    uint64_t busy = stat.user + stat.nice + stat.system + stat.irq + stat.softirq + stat.steal;
    uint64_t prev_busy = prev.user + prev.nice + prev.system + prev.irq + prev.softirq + prev.steal;

    if (total > prev_total && total > 0) {
      double usage = static_cast<double>(busy - prev_busy) / (total - prev_total) * 100.0;

      auto& sample = samples.emplace_back();
      sample.set_name("system.cpu.core.usage_percent");
      sample.set_value(usage);
      sample.set_timestamp_ms(GetCurrentTimestampMs());
      
      auto* label = sample.add_labels();
      label->set_key("core");
      label->set_value(stat.cpu_name);
    }
  }
}

void CpuMmapCollector::CollectSoftirq(std::vector<systeminsight::proto::MetricSample>& samples) {
  if (!softirq_reader_.IsValid()) return;

  auto* data = static_cast<SoftirqStatData*>(softirq_reader_.GetData());
  int count = softirq_reader_.GetValidCount();

  // 软中断总计
  uint64_t total_hi = 0, total_timer = 0, total_net_tx = 0, total_net_rx = 0;
  uint64_t total_tasklet = 0, total_sched = 0, total_rcu = 0;

  for (int i = 0; i < count; ++i) {
    const auto& stat = data[i];
    if (stat.cpu_name[0] == '\0') break;

    // 跳过聚合行
    if (stat.cpu_name[3] == '\0') continue;

    total_hi += stat.hi;
    total_timer += stat.timer;
    total_net_tx += stat.net_tx;
    total_net_rx += stat.net_rx;
    total_tasklet += stat.tasklet;
    total_sched += stat.sched;
    total_rcu += stat.rcu;
  }

  // 计算增量（需要保存历史数据）
  static uint64_t prev_hi = 0, prev_timer = 0, prev_net_tx = 0, prev_net_rx = 0;
  static uint64_t prev_tasklet = 0, prev_sched = 0, prev_rcu = 0;
  static bool has_softirq_baseline = false;

  if (has_softirq_baseline) {
    auto now = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(now - previous_sample_time_).count();
    if (elapsed > 0) {
      // 添加软中断率指标
      auto add_rate = [&](const std::string& name, uint64_t current, uint64_t prev) {
        if (current > prev) {
          auto& sample = samples.emplace_back();
          sample.set_name(name);
          sample.set_value(static_cast<double>(current - prev) / elapsed);
          sample.set_timestamp_ms(GetCurrentTimestampMs());
        }
      };

      add_rate("system.softirq.hi_per_sec", total_hi, prev_hi);
      add_rate("system.softirq.timer_per_sec", total_timer, prev_timer);
      add_rate("system.softirq.net_tx_per_sec", total_net_tx, prev_net_tx);
      add_rate("system.softirq.net_rx_per_sec", total_net_rx, prev_net_rx);
      add_rate("system.softirq.tasklet_per_sec", total_tasklet, prev_tasklet);
      add_rate("system.softirq.sched_per_sec", total_sched, prev_sched);
      add_rate("system.softirq.rcu_per_sec", total_rcu, prev_rcu);
    }
  }

  // 更新历史
  if (has_baseline_) {
    prev_hi = total_hi;
    prev_timer = total_timer;
    prev_net_tx = total_net_tx;
    prev_net_rx = total_net_rx;
    prev_tasklet = total_tasklet;
    prev_sched = total_sched;
    prev_rcu = total_rcu;
    has_softirq_baseline = true;
  }
}

double CpuMmapCollector::CalculateCpuUsage(uint64_t idle, uint64_t iowait,
                                           uint64_t total, uint64_t prev_idle,
                                           uint64_t prev_iowait, uint64_t prev_total) {
  uint64_t idle_delta = idle + iowait - prev_idle - prev_iowait;
  uint64_t total_delta = total - prev_total;

  if (total_delta == 0) return 0.0;

  return static_cast<double>(total_delta - idle_delta) / total_delta * 100.0;
}

int64_t GetCurrentTimestampMs() {
  return std::chrono::duration_cast<std::chrono::milliseconds>(
             std::chrono::system_clock::now().time_since_epoch())
      .count();
}

}  // namespace client
}  // namespace system_insight

