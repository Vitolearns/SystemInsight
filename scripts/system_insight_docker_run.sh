#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

IMAGE_NAME="${IMAGE_NAME:-system_insight:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-system_insight_dev}"
NETWORK_NAME="${NETWORK_NAME:-system_insight_net}"

# 用于内核模块加载的设备路径映射
DEV_PATH="/dev:/dev"

if ! docker network inspect "${NETWORK_NAME}" >/dev/null 2>&1; then
  echo "[system_insight] Creating docker network ${NETWORK_NAME}"
  docker network create "${NETWORK_NAME}" >/dev/null
fi

if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}$"; then
  echo "[system_insight] Removing existing container ${CONTAINER_NAME}"
  docker rm -f "${CONTAINER_NAME}" >/dev/null
fi

echo "[system_insight] Starting container ${CONTAINER_NAME}"
echo "[system_insight] NOTE: Using --privileged mode for kernel module support"
docker run -d \
  --name "${CONTAINER_NAME}" \
  --network "${NETWORK_NAME}" \
  --privileged \
  -p 50052:50052 \
  -p 9102:9102 \
  -v "${PROJECT_ROOT}:/opt/system_insight" \
  -v /lib/modules:/lib/modules:ro \
  -v /usr/src:/usr/src:ro \
  "${IMAGE_NAME}" \
  /bin/bash -c "trap 'exit 0' SIGTERM; while true; do sleep 3600; done"

echo "[system_insight] Container ${CONTAINER_NAME} is running. Use system_insight_docker_into.sh to enter."
echo ""
echo "[system_insight] To load kernel modules, run inside container:"
echo "  cd /opt/system_insight/src/kmod && make load"
