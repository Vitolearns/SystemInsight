#include "src/exporter/prometheus_exporter.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cctype>
#include <chrono>
#include <sstream>
#include <utility>

#include "src/common/logging/logging.h"

namespace system_insight {
namespace exporter {

PrometheusExporter::PrometheusExporter(std::shared_ptr<server::MetricsRepository> repository, int port)
    : repository_(std::move(repository)), port_(port) {}

PrometheusExporter::~PrometheusExporter() { Stop(); }

void PrometheusExporter::Start() {
  if (running_.exchange(true)) {
    return;
  }
  server_thread_ = std::thread(&PrometheusExporter::Serve, this);
}

void PrometheusExporter::Stop() {
  if (!running_.exchange(false)) {
    return;
  }
  if (server_fd_ >= 0) {
    shutdown(server_fd_, SHUT_RDWR);
    close(server_fd_);
    server_fd_ = -1;
  }
  if (server_thread_.joinable()) {
    server_thread_.join();
  }
}

void PrometheusExporter::Serve() {
  server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd_ < 0) {
    LOGE("Failed to create exporter socket");
    return;
  }

  int opt = 1;
  setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

  sockaddr_in addr {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = INADDR_ANY;
  addr.sin_port = htons(port_);

  if (bind(server_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    LOGE("Failed to bind exporter socket on port {}", port_);
    return;
  }

  if (listen(server_fd_, 8) < 0) {
    LOGE("Exporter socket listen failed");
    return;
  }

  LOGI("Prometheus exporter serving on 0.0.0.0:{}", port_);

  while (running_.load()) {
    int client_fd = accept(server_fd_, nullptr, nullptr);
    if (client_fd < 0) {
      if (running_.load()) {
        LOGW("Exporter accept failed");
      }
      continue;
    }

    std::string payload = BuildMetricsPayload();
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n"
             << "Content-Type: text/plain; version=0.0.4\r\n"
             << "Content-Length: " << payload.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << payload;
    auto response_str = response.str();
    send(client_fd, response_str.data(), response_str.size(), 0);
    close(client_fd);
  }
}

std::string PrometheusExporter::BuildMetricsPayload() const {
  auto snapshot = repository_->Snapshot();
  std::ostringstream ss;

  for (const auto& report : snapshot) {
    for (const auto& sample : report.samples()) {
      std::string metric_name = SanitizeMetricName(sample.name());
      ss << metric_name << "{host=\"" << EscapeLabelValue(report.host_id()) << "\"";
      for (const auto& label : sample.labels()) {
        ss << "," << SanitizeMetricName(label.key()) << "=\"" << EscapeLabelValue(label.value()) << "\"";
      }
      ss << "} " << sample.value();
      if (sample.timestamp_ms() > 0) {
        ss << " " << sample.timestamp_ms();
      }
      ss << "\n";
    }
  }

  return ss.str();
}

std::string PrometheusExporter::SanitizeMetricName(const std::string& name) {
  std::string sanitized;
  sanitized.reserve(name.size());
  for (char ch : name) {
    if (std::isalnum(static_cast<unsigned char>(ch)) || ch == '_') {
      sanitized.push_back(ch);
    } else if (ch == '.') {
      sanitized.push_back('_');
    } else {
      sanitized.push_back('_');
    }
  }
  if (sanitized.empty()) {
    sanitized = "metric";
  }
  return sanitized;
}

std::string PrometheusExporter::EscapeLabelValue(const std::string& value) {
  std::string escaped;
  escaped.reserve(value.size());
  for (char ch : value) {
    if (ch == '"' || ch == '\\' || ch == '\n') {
      escaped.push_back('\\');
    }
    if (ch == '\n') {
      escaped.push_back('n');
    } else {
      escaped.push_back(ch);
    }
  }
  return escaped;
}

}  // namespace exporter
}  // namespace system_insight

