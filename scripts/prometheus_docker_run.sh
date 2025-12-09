#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

IMAGE_NAME="${IMAGE_NAME:-prom/prometheus:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-prometheus}"
NETWORK_NAME="${NETWORK_NAME:-system_insight_net}"
CONFIG_PATH="${PROJECT_ROOT}/configs/prometheus_example.yml"

if [[ ! -f "${CONFIG_PATH}" ]]; then
  echo "[prometheus] Config file not found: ${CONFIG_PATH}" >&2
  exit 1
fi

if ! docker network inspect "${NETWORK_NAME}" >/dev/null 2>&1; then
  echo "[prometheus] Creating docker network ${NETWORK_NAME}"
  docker network create "${NETWORK_NAME}" >/dev/null
fi

if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}$"; then
  echo "[prometheus] Removing existing container ${CONTAINER_NAME}"
  docker rm -f "${CONTAINER_NAME}" >/dev/null
fi

echo "[prometheus] Starting container ${CONTAINER_NAME}"
docker run -d \
  --name "${CONTAINER_NAME}" \
  --network "${NETWORK_NAME}" \
  -p 9090:9090 \
  -v "${CONFIG_PATH}:/etc/prometheus/prometheus.yml:ro" \
  "${IMAGE_NAME}"

echo "[prometheus] Container ${CONTAINER_NAME} is running."
