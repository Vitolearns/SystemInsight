#include "src/client/metrics/mmap_reader.h"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

namespace system_insight {
namespace client {

MmapReader::MmapReader(const std::string& device_path, int max_cpus, size_t struct_size)
    : max_cpus_(max_cpus), struct_size_(struct_size), device_path_(device_path) {
  OpenAndMap();
}

MmapReader::~MmapReader() {
  if (addr_ != nullptr) {
    munmap(addr_, mapped_size_);
  }
  if (fd_ >= 0) {
    close(fd_);
  }
}

MmapReader::MmapReader(MmapReader&& other) noexcept
    : fd_(other.fd_),
      addr_(other.addr_),
      mapped_size_(other.mapped_size_),
      max_cpus_(other.max_cpus_),
      struct_size_(other.struct_size_),
      cpu_count_(other.cpu_count_),
      valid_count_(other.valid_count_),
      device_path_(std::move(other.device_path_)),
      last_error_(std::move(other.last_error_)) {
  other.fd_ = -1;
  other.addr_ = nullptr;
  other.mapped_size_ = 0;
}

MmapReader& MmapReader::operator=(MmapReader&& other) noexcept {
  if (this != &other) {
    // 清理现有资源
    if (addr_ != nullptr) {
      munmap(addr_, mapped_size_);
    }
    if (fd_ >= 0) {
      close(fd_);
    }

    // 移动资源
    fd_ = other.fd_;
    addr_ = other.addr_;
    mapped_size_ = other.mapped_size_;
    max_cpus_ = other.max_cpus_;
    struct_size_ = other.struct_size_;
    cpu_count_ = other.cpu_count_;
    valid_count_ = other.valid_count_;
    device_path_ = std::move(other.device_path_);
    last_error_ = std::move(other.last_error_);

    other.fd_ = -1;
    other.addr_ = nullptr;
    other.mapped_size_ = 0;
  }
  return *this;
}

bool MmapReader::OpenAndMap() {
  // 清理旧资源
  if (addr_ != nullptr) {
    munmap(addr_, mapped_size_);
    addr_ = nullptr;
  }
  if (fd_ >= 0) {
    close(fd_);
    fd_ = -1;
  }

  // 计算映射大小（页对齐）
  size_t total_size = struct_size_ * max_cpus_;
  size_t page_size = sysconf(_SC_PAGESIZE);
  mapped_size_ = ((total_size + page_size - 1) / page_size) * page_size;

  // 打开设备
  fd_ = open(device_path_.c_str(), O_RDONLY);
  if (fd_ < 0) {
    last_error_ = "Failed to open device: " + device_path_;
    return false;
  }

  // 执行 mmap
  addr_ = mmap(nullptr, mapped_size_, PROT_READ, MAP_SHARED, fd_, 0);
  if (addr_ == MAP_FAILED) {
    last_error_ = "mmap failed for device: " + device_path_;
    close(fd_);
    fd_ = -1;
    return false;
  }

  // 扫描有效条目数
  valid_count_ = 0;
  cpu_count_ = 0;
  char* base = static_cast<char*>(addr_);
  for (int i = 0; i < max_cpus_; ++i) {
    char* entry = base + i * struct_size_;
    if (entry[0] == '\0') {
      break;
    }
    valid_count_++;
    // 解析 CPU 编号
    if (strncmp(entry, "cpu", 3) == 0) {
      cpu_count_++;
    }
  }

  return true;
}

bool MmapReader::Refresh() {
  return OpenAndMap();
}

}  // namespace client
}  // namespace system_insight

