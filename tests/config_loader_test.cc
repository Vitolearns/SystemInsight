#include "../src/common/config/config_loader.h"

#include <filesystem>
#include <fstream>

#include <gtest/gtest.h>

namespace fs = std::filesystem;
using system_insight::common::config::ClientConfig;
using system_insight::common::config::LoadClientConfig;
using system_insight::common::config::LoadServerConfig;
using system_insight::common::config::ServerConfig;

class TempFile {
 public:
  TempFile() {
    path_ = fs::temp_directory_path() / fs::path("system_insight_test.json");
  }
  ~TempFile() { std::error_code ec; fs::remove(path_, ec); }

  const fs::path& path() const { return path_; }

 private:
  fs::path path_;
};

TEST(ConfigLoaderTest, ParsesClientConfigWithOverrides) {
  TempFile temp;
  std::ofstream out(temp.path());
  out << "{\n"
         "  \"client\": {\n"
         "    \"target\": \"10.0.0.5:6000\",\n"
         "    \"collection_interval_ms\": 2000,\n"
         "    \"log_level\": \"debug\",\n"
         "    \"host_id\": \"unit-test\"\n"
         "  }\n"
         "}\n";
  out.close();

  ClientConfig config = LoadClientConfig(temp.path());
  EXPECT_EQ(config.target, "10.0.0.5:6000");
  EXPECT_EQ(config.collection_interval_ms, 2000);
  EXPECT_EQ(config.log_level, "debug");
  EXPECT_EQ(config.host_id, "unit-test");
}

TEST(ConfigLoaderTest, ParsesServerExporterConfig) {
  TempFile temp;
  std::ofstream out(temp.path());
  out << "{\n"
         "  \"server\": {\n"
         "    \"listen_address\": \"0.0.0.0:5050\",\n"
         "    \"log_level\": \"warn\"\n"
         "  },\n"
         "  \"exporter\": {\n"
         "    \"prometheus_http_port\": 9200\n"
         "  }\n"
         "}\n";
  out.close();

  ServerConfig config = LoadServerConfig(temp.path());
  EXPECT_EQ(config.listen_address, "0.0.0.0:5050");
  EXPECT_EQ(config.log_level, "warn");
  EXPECT_EQ(config.prometheus_http_port, 9200);
}

