#ifndef SYSTEM_INSIGHT_CLIENT_SYSTEM_METRICS_COLLECTOR_H_
#define SYSTEM_INSIGHT_CLIENT_SYSTEM_METRICS_COLLECTOR_H_

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "system_insight.pb.h"
#include "src/client/metrics/cpu_mmap_collector.h"

namespace system_insight {
namespace client {

/**
 * @brief 采集器配置
 */
struct CollectorConfig {
  // mmap 采集器配置
  bool use_mmap = false;                              // 是否使用 mmap 采集
  std::string mmap_cpu_device_path = "/dev/system_insight_cpu_stat";   // CPU 统计设备路径
  std::string mmap_softirq_device_path = "/dev/system_insight_softirq"; // 软中断设备路径
};

/**
 * @brief 系统指标采集器
 * 
 * 统一的系统指标采集接口，支持两种采集方式：
 * 1. mmap 模式：使用内核模块 + 共享内存（高性能）
 * 2. /proc 模式：读取 /proc 文件系统（通用模式）
 * 
 * 优先尝试 mmap 模式，如果内核模块不可用则自动回退到 /proc 模式。
 */
class SystemMetricsCollector {
 public:
  /**
   * @brief 构造函数
   * @param config 采集器配置
   */
  explicit SystemMetricsCollector(const CollectorConfig& config = CollectorConfig());

  /**
   * @brief 采集所有系统指标
   * @return 采集到的指标样本向量
   */
  std::vector<systeminsight::proto::MetricSample> Collect();

  /**
   * @brief 检查当前使用的采集模式
   * @return true 表示使用 mmap 模式
   */
  bool IsUsingMmap() const { return use_mmap_; }

  /**
   * @brief 检查采集器是否可用
   */
  bool IsAvailable() const {
    return use_mmap_ ? (mmap_collector_ != nullptr) : true;
  }

 private:
  void CollectCpuUsage(std::vector<systeminsight::proto::MetricSample>& samples);
  void CollectMemInfo(std::vector<systeminsight::proto::MetricSample>& samples);
  void CollectNetDev(std::vector<systeminsight::proto::MetricSample>& samples);

  bool ReadCpuTimes(uint64_t* idle, uint64_t* total) const;
  bool ReadMemInfo(uint64_t* total, uint64_t* available) const;
  bool ReadNetDev(uint64_t* rx_bytes, uint64_t* tx_bytes) const;

  // 配置
  CollectorConfig config_;

  // mmap 采集器（可选）
  std::unique_ptr<CpuMmapCollector> mmap_collector_;

  // 历史数据
  std::chrono::steady_clock::time_point previous_sample_time_;
  uint64_t prev_cpu_idle_ = 0;
  uint64_t prev_cpu_total_ = 0;
  uint64_t prev_rx_bytes_ = 0;
  uint64_t prev_tx_bytes_ = 0;

  // 状态标记
  bool use_mmap_ = false;
  bool has_cpu_baseline_ = false;
  bool has_net_baseline_ = false;
};

}  // namespace client
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_CLIENT_SYSTEM_METRICS_COLLECTOR_H_
