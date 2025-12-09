#!/usr/bin/env bash
set -euo pipefail

CONTAINER_NAME="${CONTAINER_NAME:-prometheus}"

if docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}$"; then
  echo "[prometheus] Stopping and removing container ${CONTAINER_NAME}"
  docker rm -f "${CONTAINER_NAME}" >/dev/null
  echo "[prometheus] Container ${CONTAINER_NAME} removed."
else
  echo "[prometheus] Container ${CONTAINER_NAME} does not exist, nothing to do."
fi
