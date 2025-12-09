#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

IMAGE_NAME="${IMAGE_NAME:-system_insight:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-system_insight_dev}"
NETWORK_NAME="${NETWORK_NAME:-system_insight_net}"

if ! docker network inspect "${NETWORK_NAME}" >/dev/null 2>&1; then
  echo "[system_insight] Creating docker network ${NETWORK_NAME}"
  docker network create "${NETWORK_NAME}" >/dev/null
fi

if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}$"; then
  echo "[system_insight] Removing existing container ${CONTAINER_NAME}"
  docker rm -f "${CONTAINER_NAME}" >/dev/null
fi

echo "[system_insight] Starting container ${CONTAINER_NAME}"
docker run -d \
  --name "${CONTAINER_NAME}" \
  --network "${NETWORK_NAME}" \
  -p 50052:50052 \
  -p 9102:9102 \
  -v "${PROJECT_ROOT}:/opt/system_insight" \
  "${IMAGE_NAME}" \
  /bin/bash -c "trap 'exit 0' SIGTERM; while true; do sleep 3600; done"

echo "[system_insight] Container ${CONTAINER_NAME} is running. Use system_insight_docker_into.sh to enter."
