#!/usr/bin/env bash
set -euo pipefail

IMAGE_NAME="${IMAGE_NAME:-grafana/grafana:latest}"
CONTAINER_NAME="${CONTAINER_NAME:-grafana}"
NETWORK_NAME="${NETWORK_NAME:-system_insight_net}"

if ! docker network inspect "${NETWORK_NAME}" >/dev/null 2>&1; then
  echo "[grafana] Creating docker network ${NETWORK_NAME}"
  docker network create "${NETWORK_NAME}" >/dev/null
fi

if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}$"; then
  echo "[grafana] Removing existing container ${CONTAINER_NAME}"
  docker rm -f "${CONTAINER_NAME}" >/dev/null
fi

echo "[grafana] Starting container ${CONTAINER_NAME}"
docker run -d \
  --name "${CONTAINER_NAME}" \
  --network "${NETWORK_NAME}" \
  -p 3000:3000 \
  "${IMAGE_NAME}"

echo "[grafana] Container ${CONTAINER_NAME} is running. Configure Prometheus URL as http://prometheus:9090 inside Grafana."
