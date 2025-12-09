#!/usr/bin/env bash
set -euo pipefail

CONTAINER_NAME="${CONTAINER_NAME:-grafana}"

if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}$"; then
  echo "[grafana] Stopping and removing container ${CONTAINER_NAME}"
  docker rm -f "${CONTAINER_NAME}" >/dev/null
  echo "[grafana] Container ${CONTAINER_NAME} removed."
else
  echo "[grafana] Container ${CONTAINER_NAME} does not exist, nothing to do."
fi
