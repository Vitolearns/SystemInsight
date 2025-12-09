#!/usr/bin/env bash
set -euo pipefail

ARCHIVE_ROOT="${ARCHIVE_ROOT:-/opt/system_insight/offline/archives}"
BUILD_ROOT="${BUILD_ROOT:-/tmp/system_insight_offline_build}"
INSTALL_PREFIX="${INSTALL_PREFIX:-/usr/local}"
CMAKE_VERSION="${CMAKE_VERSION:-3.27.6}"
NINJA_VERSION="${NINJA_VERSION:-1.11.1}"
ABSL_VERSION="${ABSL_VERSION:-20240116.0}"
PROTOBUF_VERSION="${PROTOBUF_VERSION:-25.3}"
GRPC_VERSION="${GRPC_VERSION:-1.62.0}"

NUM_JOBS="${NUM_JOBS:-$(nproc)}"

mkdir -p "${BUILD_ROOT}"

require_archive() {
  local file="$1"
  local path="${ARCHIVE_ROOT}/${file}"
  if [[ ! -f "${path}" ]]; then
    echo "[offline-install] Missing archive: ${path}" >&2
    exit 1
  fi
  printf '%s' "${path}"
}

install_cmake() {
  local archive
  archive="$(require_archive "cmake-${CMAKE_VERSION}-linux-x86_64.tar.gz")"
  echo "[offline-install] Installing CMake ${CMAKE_VERSION}"
  tar -xf "${archive}" -C /opt
  ln -sfn "/opt/cmake-${CMAKE_VERSION}-linux-x86_64" /opt/cmake
  ln -sf /opt/cmake/bin/cmake /usr/local/bin/cmake
  ln -sf /opt/cmake/bin/ctest /usr/local/bin/ctest
  ln -sf /opt/cmake/bin/cpack /usr/local/bin/cpack
}

install_ninja() {
  local archive
  archive="$(require_archive "ninja-linux-${NINJA_VERSION}.zip")"
  echo "[offline-install] Installing Ninja ${NINJA_VERSION}"
  unzip -o "${archive}" -d /usr/local/bin >/dev/null
  chmod +x /usr/local/bin/ninja
}

install_absl() {
  local archive
  archive="$(require_archive "abseil-cpp-${ABSL_VERSION}.tar.gz")"
  echo "[offline-install] Installing abseil ${ABSL_VERSION}"
  local src="${BUILD_ROOT}/absl-src"
  rm -rf "${src}"
  mkdir -p "${src}"
  tar -xf "${archive}" -C "${src}" --strip-components=1
  cmake_build_install "${src}" "${src}/build" \
    -DABSL_PROPAGATE_CXX_STD=ON
}

cmake_build_install() {
  local src_dir="$1"
  local build_dir="$2"
  shift 2
  cmake -S "${src_dir}" -B "${build_dir}" -DCMAKE_BUILD_TYPE=Release -DCMAKE_POSITION_INDEPENDENT_CODE=ON "$@"
  cmake --build "${build_dir}" -j "${NUM_JOBS}"
  cmake --install "${build_dir}"
}

install_protobuf() {
  local archive
  archive="$(require_archive "protobuf-v${PROTOBUF_VERSION}-with-deps.tar.gz")"
  echo "[offline-install] Installing Protobuf ${PROTOBUF_VERSION}"
  local src="${BUILD_ROOT}/protobuf-src"
  rm -rf "${src}"
  mkdir -p "${src}"
  tar -xf "${archive}" -C "${src}"
  cmake_build_install "${src}" "${src}/build" \
    -Dprotobuf_BUILD_TESTS=OFF \
    -Dprotobuf_ABSL_PROVIDER=none
}

install_grpc() {
  local archive
  archive="$(require_archive "grpc-v${GRPC_VERSION}-with-deps.tar.gz")"
  echo "[offline-install] Installing gRPC ${GRPC_VERSION}"
  local src="${BUILD_ROOT}/grpc-src"
  rm -rf "${src}"
  mkdir -p "${src}"
  tar -xf "${archive}" -C "${src}"
  cmake_build_install "${src}" "${src}/build" \
    -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_INSTALL=ON \
    -DgRPC_PROTOBUF_PROVIDER=package \
    -DProtobuf_DIR="${INSTALL_PREFIX}/lib/cmake/protobuf" \
    -DgRPC_ABSL_PROVIDER=package \
    -Dabsl_DIR="${INSTALL_PREFIX}/lib/cmake/absl" \
    -DgRPC_ZLIB_PROVIDER=module \
    -DgRPC_CARES_PROVIDER=module \
    -DgRPC_RE2_PROVIDER=module \
    -DgRPC_SSL_PROVIDER=package
}

install_spdlog() {
  local archive
  archive="$(require_archive "spdlog-1.13.0.tar.gz")"
  echo "[offline-install] Installing spdlog 1.13.0"
  local src="${BUILD_ROOT}/spdlog-src"
  rm -rf "${src}"
  mkdir -p "${src}"
  tar -xf "${archive}" -C "${src}" --strip-components=1
  cmake_build_install "${src}" "${src}/build" -DSPDLOG_BUILD_TESTS=OFF -DSPDLOG_BUILD_EXAMPLE=OFF
}

install_gflags() {
  local archive
  archive="$(require_archive "gflags-2.2.2.tar.gz")"
  echo "[offline-install] Installing gflags 2.2.2"
  local src="${BUILD_ROOT}/gflags-src"
  rm -rf "${src}"
  mkdir -p "${src}"
  tar -xf "${archive}" -C "${src}" --strip-components=1
  cmake_build_install "${src}" "${src}/build" -DBUILD_SHARED_LIBS=ON
}

install_json() {
  local archive
  archive="$(require_archive "nlohmann_json-3.11.3.tar.gz")"
  echo "[offline-install] Installing nlohmann_json 3.11.3"
  local src="${BUILD_ROOT}/json-src"
  rm -rf "${src}"
  mkdir -p "${src}"
  tar -xf "${archive}" -C "${src}" --strip-components=1
  cmake_build_install "${src}" "${src}/build"
}

install_googletest() {
  local archive
  archive="$(require_archive "googletest-1.14.0.tar.gz")"
  echo "[offline-install] Installing googletest 1.14.0"
  local src="${BUILD_ROOT}/gtest-src"
  rm -rf "${src}"
  mkdir -p "${src}"
  tar -xf "${archive}" -C "${src}" --strip-components=1
  cmake_build_install "${src}" "${src}/build" -DBUILD_GMOCK=ON -DBUILD_GTEST=ON
}

echo "[offline-install] Using archives from ${ARCHIVE_ROOT}"
install_cmake
install_ninja
install_absl
install_protobuf
install_grpc
install_spdlog
install_gflags
install_json
install_googletest

rm -rf "${BUILD_ROOT}"
echo "[offline-install] All dependencies installed to ${INSTALL_PREFIX}"

