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
        cmake \
        ninja-build \
        kmod \
        sudo \
        libprotobuf-dev \
        protobuf-compiler \
        libgrpc++-dev \
        libgrpc-dev \
        protobuf-compiler-grpc \
        libspdlog-dev \
        libgflags-dev \
        nlohmann-json3-dev \
        libgtest-dev \
        libssl-dev \
        linux-headers-$(uname -r) \
        linux-headers-generic && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /opt/system_insight

COPY . .

ENV OFFLINE_ROOT=/opt/system_insight/third_party/offline
ENV PATH="/opt/cmake/bin:${PATH}"

EXPOSE 50052 9102
CMD ["/bin/bash"]
