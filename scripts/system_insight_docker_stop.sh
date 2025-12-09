#!/usr/bin/env bash
set -euo pipefail

CONTAINER_NAME="${CONTAINER_NAME:-system_insight_dev}"

if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}\$"; then
  echo "[system_insight] Stopping and removing container ${CONTAINER_NAME}"
  docker rm -f "${CONTAINER_NAME}" >/dev/null
  echo "[system_insight] Container ${CONTAINER_NAME} removed."
else
  echo "[system_insight] Container ${CONTAINER_NAME} does not exist, nothing to do."
fi


