#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"
ARCHIVE_DIR="${PROJECT_ROOT}/third_party/offline/archives"

IMAGE_NAME="${IMAGE_NAME:-system_insight:latest}"

REQUIRED_ARCHIVES=(
  "cmake-3.27.6-linux-x86_64.tar.gz"
  "ninja-linux-1.11.1.zip"
  "abseil-cpp-20240116.0.tar.gz"
  "protobuf-v25.3-with-deps.tar.gz"
  "grpc-v1.62.0-with-deps.tar.gz"
  "spdlog-1.13.0.tar.gz"
  "gflags-2.2.2.tar.gz"
  "nlohmann_json-3.11.3.tar.gz"
  "googletest-1.14.0.tar.gz"
)

for archive in "${REQUIRED_ARCHIVES[@]}"; do
  if [[ ! -f "${ARCHIVE_DIR}/${archive}" ]]; then
    echo "[system_insight] Missing ${archive} in ${ARCHIVE_DIR}"
    echo "[system_insight] Please run scripts/system_insight_offline_prepare.sh before building."
    exit 1
  fi
done

echo "[system_insight] Building Docker image ${IMAGE_NAME}"
docker build \
  -t "${IMAGE_NAME}" \
  -f "${PROJECT_ROOT}/docker/Dockerfile" \
  "${PROJECT_ROOT}"

echo "[system_insight] Docker image ${IMAGE_NAME} build completed."

