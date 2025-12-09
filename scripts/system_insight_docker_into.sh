#!/usr/bin/env bash
set -euo pipefail

CONTAINER_NAME="${CONTAINER_NAME:-system_insight_dev}"

if ! docker ps --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}\$"; then
  echo "Container ${CONTAINER_NAME} is not running. Please run system_insight_docker_run.sh first." >&2
  exit 1
fi

docker exec -it "${CONTAINER_NAME}" /bin/bash

