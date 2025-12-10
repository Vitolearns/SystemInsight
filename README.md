# system_insight

`system_insight` 是一个面向 Linux 系统/应用监控的 gRPC + Grafana 方案

- 采集主机/业务指标，通过 gRPC client → server 传输
- 服务端聚合并通过 Prometheus metrics 暴露，Grafana 看板展示
- 全流程统一使用 CMake 构建体系
- 编译、运行均可在 Docker 容器中完成

## 项目结构

```
.
├── CMakeLists.txt       # 顶层 CMake 入口
├── configs/             # 各类配置文件
├── docker/              # Dockerfile 等
├── scripts/             # 构建 / 容器相关脚本
├── src/
│   ├── client/          # gRPC client & 指标采集框架
│   ├── common/          # 工具、日志、配置解析
│   ├── exporter/        # Prometheus/Grafana 对接C++程序的接口
│   ├── proto/           # .proto 及生成规则
│   └── server/          # gRPC server + 聚合逻辑
├── tests/               # 单元 & 集成测试
```

## 快速开始（全部在 Docker 中完成）

0. **联网环境离线准备一次**
   ```bash
   ./scripts/system_insight_offline_prepare.sh
   # third_party/offline/archives/ 包含 cmake / protobuf / grpc 等离线包
   ```
1. **构建镜像（仅读取本地离线包）**
   ```bash
   ./scripts/system_insight_docker_build.sh
   # docker/Dockerfile 会将 third_party/offline/ 拷入镜像
   # 并通过 installers/install_all.sh 安装所有依赖
   ```
2. **拉起各个容器（system_insight + Prometheus + Grafana）**
   所有监控栈的容器加入同一自定义网络 `system_insight_net`，互通固定：
   ```bash
   ./scripts/system_insight_docker_run.sh   # 启动 system_insight_dev（含源码挂载，暴露 50052/9102）
   ./scripts/prometheus_docker_run.sh       # 启动 prometheus，抓取 system_insight_dev:9102
   ./scripts/grafana_docker_run.sh          # 启动 grafana，数据源指向 http://prometheus:9090
   ```
   停止对应容器：
   ```bash
   ./scripts/system_insight_docker_stop.sh
   ./scripts/prometheus_docker_stop.sh
   ./scripts/grafana_docker_stop.sh
   ```
3. **进入 system_insight 容器并编译 / 运行 server & client**
   ```bash
   ./scripts/system_insight_docker_into.sh
   cd /opt/system_insight

   # 编译
   cmake -S . -B build && cmake --build build

   # 运行 server / client
   ./build/src/server/system_insight_server --config=configs/server_example.json
   ./build/src/client/system_insight_client --config=configs/client_example.json
   ```
4. **配置文件**
   - `configs/server_example.json`：gRPC 监听、日志级别、Prometheus exporter 端口
   - `configs/client_example.json`：采集周期、目标地址、日志级别、host id

所有二进制均通过 `gflags` 暴露 `--config=/path/to/json` 参数，服务端/客户端可在本地或容器内自由切换配置，实现环境隔离。

客户端读取 `/proc/stat`、`/proc/meminfo`、`/proc/net/dev` 采集 CPU / 内存 / 网络指标，通过 gRPC 向服务端上报；服务端接收后存入指标仓库，后续会通过 Prometheus exporter 对 Grafana 暴露。


## Prometheus / Grafana

- 通过脚本启动后，容器名固定：`prometheus`(9090)、`grafana`(3000)。
- Prometheus 抓取配置：`configs/prometheus_example.yml`（默认抓取 `system_insight_dev:9102/metrics`）
- 内置 Prometheus exporter 端口在 `server_config.exporter.prometheus_http_port`（默认 `9102`）
- server跑起来后，可以通过curl -v http://localhost:9102来测试看能否GET到原始数据
- Grafana 使用方式：
  1. 浏览器访问 `http://localhost:3000` 登录 Grafana, 默认admin admin；
  2. 在 **Connections → Data sources** 中新增 Prometheus 数据源，URL 使用 `http://prometheus:9090`，保存并测试通过；
  3. 在 **Dashboards → New → Import** 中导入 `docs/grafana_system_insight_dashboard.json`；
  4. 选择上一步配置的 Prometheus 数据源，即可看到：
     - 顶部 CPU / 内存使用率总览 Stat 卡片；
     - 按行展示的 CPU / Memory / Network 时序图；
     - 最近一次采样的指标表格。

## 研发规范

- 代码遵循 Google C++ Style，使用 `.clang-format` 自动化格式化
- 日志统一封装在 `src/common/logging`，基于 `spdlog`
- 所有可视化依赖 Grafana + Prometheus
