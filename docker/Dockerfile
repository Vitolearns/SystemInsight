# syntax=docker/dockerfile:1.6

FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && \
    apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        curl \
        git \
        pkg-config \
        python3 \
        python3-pip \
        unzip \
        protobuf-compiler \
        libprotobuf-dev \
        libgrpc++-dev \
        libgrpc-dev \
        libspdlog-dev \
        libgflags-dev \
        nlohmann-json3-dev \
        libssl-dev && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt/system_insight

COPY . .

ENV OFFLINE_ROOT=/opt/system_insight/third_party/offline
RUN ARCHIVE_ROOT="${OFFLINE_ROOT}/archives" \
    BUILD_ROOT="/tmp/system_insight_offline_build" \
    INSTALL_PREFIX="/usr/local" \
    "${OFFLINE_ROOT}/installers/install_all.sh"

ENV PATH="/opt/cmake/bin:${PATH}"

EXPOSE 50052 9102
CMD ["/bin/bash"]

