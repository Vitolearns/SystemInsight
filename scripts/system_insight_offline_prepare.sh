#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ARCHIVE_DIR="${REPO_ROOT}/third_party/offline/archives"
SRC_DIR="${REPO_ROOT}/third_party/offline/src"

mkdir -p "${ARCHIVE_DIR}" "${SRC_DIR}"

require_cmd() {
  command -v "$1" >/dev/null 2>&1 || {
    echo "[offline-prepare] Missing required command: $1" >&2
    exit 1
  }
}

require_cmd curl
require_cmd sha256sum
require_cmd git

download() {
  local url="$1"
  local output="${ARCHIVE_DIR}/$2"
  local checksum="$3"
  if [[ -f "${output}" ]]; then
    echo "[offline-prepare] ${output##*/} already exists, skipping download."
    return
  fi
  echo "[offline-prepare] Downloading ${output##*/}"
  curl -L --retry 5 --retry-delay 2 "${url}" -o "${output}"
  echo "${checksum}  ${output##*/}" | (cd "${ARCHIVE_DIR}" && sha256sum --check)
}

prepare_grpc() {
  local version="v1.62.0"
  local archive="${ARCHIVE_DIR}/grpc-${version}-with-deps.tar.gz"
  if [[ -f "${archive}" ]]; then
    echo "[offline-prepare] ${archive##*/} already exists, skipping."
    return
  fi
  echo "[offline-prepare] Cloning grpc ${version} with submodules..."
  local clone_dir="${SRC_DIR}/grpc"
  rm -rf "${clone_dir}"
  git clone --depth 1 --branch "${version}" --recurse-submodules https://github.com/grpc/grpc.git "${clone_dir}"
  echo "[offline-prepare] Archiving gRPC with submodules."
  tar -czf "${archive}" -C "${clone_dir}" .
  rm -rf "${clone_dir}"
}

prepare_protobuf() {
  local version="v25.3"
  local archive="${ARCHIVE_DIR}/protobuf-${version}-with-deps.tar.gz"
  if [[ -f "${archive}" ]]; then
    echo "[offline-prepare] ${archive##*/} already exists, skipping."
    return
  fi
  echo "[offline-prepare] Cloning protobuf ${version} with submodules..."
  local clone_dir="${SRC_DIR}/protobuf"
  rm -rf "${clone_dir}"
  git clone --depth 1 --branch "${version}" https://github.com/protocolbuffers/protobuf.git "${clone_dir}"
  (cd "${clone_dir}" && git submodule update --init --recursive)
  echo "[offline-prepare] Archiving protobuf with deps."
  tar -czf "${archive}" -C "${clone_dir}" .
  rm -rf "${clone_dir}"
}

# URLs & checksums
download "https://github.com/Kitware/CMake/releases/download/v3.27.6/cmake-3.27.6-linux-x86_64.tar.gz" \
  "cmake-3.27.6-linux-x86_64.tar.gz" \
  "9a02001215df244963b15c1022d0782d6132ec7ce65c69a776c60f44aea99e79"

download "https://github.com/ninja-build/ninja/releases/download/v1.11.1/ninja-linux.zip" \
  "ninja-linux-1.11.1.zip" \
  "b901ba96e486dce377f9a070ed4ef3f79deb45f4ffe2938f8e7ddc69cfb3df77"

download "https://github.com/gabime/spdlog/archive/refs/tags/v1.13.0.tar.gz" \
  "spdlog-1.13.0.tar.gz" \
  "a8cbfbc22043343180896ac0e3abc92d7fb6292a3a3aad0cb1afd61bf3eef21f"

download "https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.tar.gz" \
  "gflags-2.2.2.tar.gz" \
  "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf"

download "https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz" \
  "nlohmann_json-3.11.3.tar.gz" \
  "0d8ef5af7f9794e3263480193c491549b2ba6cc74bb018906202ada498a79406"

download "https://github.com/google/googletest/archive/refs/tags/v1.14.0.tar.gz" \
  "googletest-1.14.0.tar.gz" \
  "8ad598c73ad796e0d8280b082cebd82a630d73e73cd3c70057938a6501bba5d7"

download "https://github.com/abseil/abseil-cpp/archive/refs/tags/20240116.0.tar.gz" \
  "abseil-cpp-20240116.0.tar.gz" \
  "338420448b140f0dfd1a1ea3c3ce71b3bc172071f24f4d9a57d59b45037da440"

prepare_protobuf
prepare_grpc

echo "[offline-prepare] Offline artifacts stored at ${ARCHIVE_DIR}"

