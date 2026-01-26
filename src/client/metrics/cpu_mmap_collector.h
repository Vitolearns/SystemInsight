#ifndef SYSTEM_INSIGHT_CLIENT_CPU_MMAP_COLLECTOR_H_
#define SYSTEM_INSIGHT_CLIENT_CPU_MMAP_COLLECTOR_H_

#include <chrono>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "src/client/metrics/mmap_reader.h"
#include "system_insight.pb.h"

namespace system_insight {
namespace client {

// Maximum number of CPUs supported
constexpr int MAX_CPUS = 256;

/**
 * @brief 基于 mmap 的 CPU 指标采集器
 * 
 * 通过 mmap 直接读取内核模块的共享内存，获取 per-CPU 核心的详细统计。
 * 相比传统的 /proc/stat 读取方式，具有以下优势：
 * - 零拷贝：无内核→用户空间数据拷贝
 * - 更细粒度：获取每个 CPU 核心的独立统计
 * - 实时性：内核每秒自动更新数据
 */
class CpuMmapCollector {
 public:
  /**
   * @brief 构造函数
   * @param device_path CPU 统计设备路径
   * @param softirq_device_path 软中断设备路径（可选）
   */
  explicit CpuMmapCollector(const std::string& device_path,
                            const std::string& softirq_device_path = "");

  /**
   * @brief 采集所有 CPU 指标
   * @return 采集到的指标样本向量
   */
  std::vector<systeminsight::proto::MetricSample> Collect();

  /**
   * @brief 检查采集器是否可用（内核模块是否已加载）
   */
  bool IsAvailable() const {
    return cpu_reader_.IsValid() || softirq_reader_.IsValid();
  }

  /**
   * @brief 获取最后一次错误信息
   */
  std::string GetLastError() const {
    if (!cpu_reader_.IsValid()) return cpu_reader_.GetLastError();
    if (!softirq_reader_.GetLastError().empty()) return softirq_reader_.GetLastError();
    return "";
  }

 private:
  /**
   * @brief 采集 CPU 使用率指标
   */
  void CollectCpuUsage(std::vector<systeminsight::proto::MetricSample>& samples);

  /**
   * @brief 采集 per-CPU 核心指标
   */
  void CollectPerCpuCore(std::vector<systeminsight::proto::MetricSample>& samples);

  /**
   * @brief 采集软中断指标
   */
  void CollectSoftirq(std::vector<systeminsight::proto::MetricSample>& samples);

  /**
   * @brief 计算 CPU 使用率百分比
   */
  double CalculateCpuUsage(uint64_t idle, uint64_t iowait,
                          uint64_t total, uint64_t prev_idle, uint64_t prev_iowait,
                          uint64_t prev_total);

  MmapReader cpu_reader_;
  MmapReader softirq_reader_;
  std::string softirq_device_path_;
  
  // 历史数据缓存（用于计算增量）
  std::unordered_map<std::string, CpuStatData> prev_cpu_stats_;
  std::unordered_map<std::string, SoftirqStatData> prev_softirq_stats_;
  
  std::chrono::steady_clock::time_point previous_sample_time_;
  bool has_baseline_ = false;
};

}  // namespace client
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_CLIENT_CPU_MMAP_COLLECTOR_H_

