#ifndef SYSTEM_INSIGHT_CLIENT_MMAP_READER_H_
#define SYSTEM_INSIGHT_CLIENT_MMAP_READER_H_

#include <cstddef>
#include <string>
#include <optional>

namespace system_insight {
namespace client {

/**
 * @brief 通用 mmap 读取器
 * 
 * 用于从内核模块的共享内存设备读取数据的通用接口。
 * 通过 mmap 直接访问内核分配的高速共享内存，
 * 实现零拷贝数据读取。
 */
class MmapReader {
 public:
  /**
   * @brief 构造函数
   * @param device_path 设备文件路径（如 /dev/system_insight_cpu_stat）
   * @param max_cpus 最大 CPU 数量
   * @param struct_size 单个数据结构体大小
   */
  MmapReader(const std::string& device_path, int max_cpus, size_t struct_size);
  
  /**
   * @brief 析构函数 - 自动清理资源
   */
  ~MmapReader();

  // 禁止拷贝
  MmapReader(const MmapReader&) = delete;
  MmapReader& operator=(const MmapReader&) = delete;

  // 允许移动
  MmapReader(MmapReader&&) noexcept;
  MmapReader& operator=(MmapReader&&) noexcept;

  /**
   * @brief 检查设备是否已打开
   */
  bool IsValid() const { return fd_ >= 0 && addr_ != nullptr; }

  /**
   * @brief 获取 mmap 起始地址
   */
  void* GetData() const { return addr_; }

  /**
   * @brief 获取实际 CPU 数量
   */
  int GetCpuCount() const { return cpu_count_; }

  /**
   * @brief 获取最后一个有效条目的索引
   */
  int GetValidCount() const { return valid_count_; }

  /**
   * @brief 获取错误信息
   */
  std::string GetLastError() const { return last_error_; }

  /**
   * @brief 重新打开并映射设备
   * @return 是否成功
   */
  bool Refresh();

 private:
  bool OpenAndMap();

  int fd_ = -1;                    // 设备文件描述符
  void* addr_ = nullptr;           // mmap 映射地址
  size_t mapped_size_ = 0;         // 映射大小
  int max_cpus_;                   // 最大 CPU 数量
  size_t struct_size_;             // 单个结构体大小
  int cpu_count_ = 0;              // 实际 CPU 数量
  int valid_count_ = 0;            // 有效条目数
  std::string device_path_;        // 设备路径
  std::string last_error_;         // 最后错误信息
};

/**
 * @brief CPU 统计结构体（与内核模块保持一致）
 */
struct CpuStatData {
  char cpu_name[16];
  uint64_t user;
  uint64_t nice;
  uint64_t system;
  uint64_t idle;
  uint64_t iowait;
  uint64_t irq;
  uint64_t softirq;
  uint64_t steal;
  uint64_t guest;
  uint64_t guest_nice;
};

/**
 * @brief 软中断统计结构体（与内核模块保持一致）
 */
struct SoftirqStatData {
  char cpu_name[16];
  uint64_t hi;
  uint64_t timer;
  uint64_t net_tx;
  uint64_t net_rx;
  uint64_t block;
  uint64_t irq_poll;
  uint64_t tasklet;
  uint64_t sched;
  uint64_t hrtimer;
  uint64_t rcu;
};

}  // namespace client
}  // namespace system_insight

#endif  // SYSTEM_INSIGHT_CLIENT_MMAP_READER_H_

