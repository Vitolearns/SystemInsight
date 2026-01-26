#include "src/common/config/config_loader.h"

#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unistd.h>

#include "nlohmann/json.hpp"
#include "src/common/logging/logging.h"

namespace system_insight {
namespace common {
namespace config {

namespace {

using json = nlohmann::json;

json ParseJsonFile(const std::string& path) {
  std::ifstream file(path);
  if (!file.is_open()) {
    LOGW("Config file {} not found, using defaults", path);
    return json::object();
  }
  try {
    json doc = json::parse(file, nullptr, true, true);
    if (!doc.is_object()) {
      LOGW("Config file {} does not contain a JSON object at the root", path);
      return json::object();
    }
    return doc;
  } catch (const std::exception& ex) {
    LOGW("Failed to parse JSON config {}: {}", path, ex.what());
    return json::object();
  }
}

std::string DetectHostId() {
  char buffer[256];
  if (gethostname(buffer, sizeof(buffer)) == 0) {
    return std::string(buffer);
  }
  return "unknown-host";
}

int ToIntOrDefault(const json& object, const char* key, int fallback) {
  if (auto it = object.find(key); it != object.end()) {
    if (it->is_number_integer()) {
      return it->get<int>();
    }
    if (it->is_number_float()) {
      return static_cast<int>(it->get<double>());
    }
    if (it->is_string()) {
      try {
        return std::stoi(it->get<std::string>());
      } catch (const std::exception&) {
        return fallback;
      }
    }
  }
  return fallback;
}

}  // namespace

ClientConfig LoadClientConfig(const std::string& path) {
  ClientConfig config;
  auto doc = ParseJsonFile(path);
  if (auto it = doc.find("client"); it != doc.end() && it->is_object()) {
    const auto& client_section = *it;
    if (auto target = client_section.find("target"); target != client_section.end() && target->is_string()) {
      config.target = target->get<std::string>();
    } else {
      LOGW("client.target not found or not a string, using default");
    }
    config.collection_interval_ms =
        ToIntOrDefault(client_section, "collection_interval_ms", config.collection_interval_ms);
    if (auto log_level = client_section.find("log_level"); log_level != client_section.end() && log_level->is_string()) {
      config.log_level = log_level->get<std::string>();
    } else {
      LOGW("client.log_level not found or not a string, using default");
    }
    if (auto host_id = client_section.find("host_id"); host_id != client_section.end() && host_id->is_string()) {
      config.host_id = host_id->get<std::string>();
    } else {
      LOGW("client.host_id not found or not a string, will use detected hostname");
    }
    
    // mmap 采集器配置
    if (auto use_mmap = client_section.find("use_mmap"); use_mmap != client_section.end()) {
      if (use_mmap->is_boolean()) {
        config.use_mmap = use_mmap->get<bool>();
      }
    }
    if (auto mmap_cpu_path = client_section.find("mmap_cpu_device_path"); 
        mmap_cpu_path != client_section.end() && mmap_cpu_path->is_string()) {
      config.mmap_cpu_device_path = mmap_cpu_path->get<std::string>();
    }
    if (auto mmap_softirq_path = client_section.find("mmap_softirq_device_path"); 
        mmap_softirq_path != client_section.end() && mmap_softirq_path->is_string()) {
      config.mmap_softirq_device_path = mmap_softirq_path->get<std::string>();
    }
  } else {
    LOGW("client section not found or not an object in config, using defaults");
  }
  if (config.host_id.empty()) {
    config.host_id = DetectHostId();
  }
  return config;
}

ServerConfig LoadServerConfig(const std::string& path) {
  ServerConfig config;
  auto doc = ParseJsonFile(path);
  if (auto it = doc.find("server"); it != doc.end() && it->is_object()) {
    const auto& server_section = *it;
    if (auto listen_address = server_section.find("listen_address");
        listen_address != server_section.end() && listen_address->is_string()) {
      config.listen_address = listen_address->get<std::string>();
    } else {
      LOGW("server.listen_address not found or not a string, using default");
    }
    if (auto log_level = server_section.find("log_level"); log_level != server_section.end() && log_level->is_string()) {
      config.log_level = log_level->get<std::string>();
    } else {
      LOGW("server.log_level not found or not a string, using default");
    }
  } else {
    LOGW("server section not found or not an object in config, using defaults");
  }
  if (auto exporter_it = doc.find("exporter"); exporter_it != doc.end() && exporter_it->is_object()) {
    config.prometheus_http_port =
        ToIntOrDefault(*exporter_it, "prometheus_http_port", config.prometheus_http_port);
  } else {
    LOGW("exporter section not found or not an object in config, using default prometheus_http_port");
  }
  return config;
}

}  // namespace config
}  // namespace common
}  // namespace system_insight

