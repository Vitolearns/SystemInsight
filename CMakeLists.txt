cmake_minimum_required(VERSION 3.20)

project(system_insight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Use pkg-config for system-installed packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(Protobuf REQUIRED protobuf)
pkg_check_modules(GRPC REQUIRED grpc++)
pkg_check_modules(GFLAGS REQUIRED gflags)
pkg_check_modules(SPDLOG REQUIRED spdlog)
pkg_check_modules(nlohmann_json REQUIRED nlohmann_json)

# Find other system packages
find_package(GTest)

# Create imported targets from pkg-config variables
add_library(grpc::grpc++ INTERFACE IMPORTED)
set_target_properties(grpc::grpc++ PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GRPC_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${GRPC_LIBRARIES}"
)

add_library(protobuf::libprotobuf INTERFACE IMPORTED)
set_target_properties(protobuf::libprotobuf PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${Protobuf_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${Protobuf_LIBRARIES}"
)

add_library(gflags::gflags INTERFACE IMPORTED)
set_target_properties(gflags::gflags PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GFLAGS_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${GFLAGS_LIBRARIES}"
)

add_library(spdlog::spdlog INTERFACE IMPORTED)
set_target_properties(spdlog::spdlog PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${SPDLOG_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${SPDLOG_LIBRARIES}"
)

add_library(nlohmann_json::nlohmann_json INTERFACE IMPORTED)
set_target_properties(nlohmann_json::nlohmann_json PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${nlohmann_json_INCLUDE_DIRS}"
    INTERFACE_LINK_LIBRARIES "${nlohmann_json_LIBRARIES}"
)

# Find absl (may not be available on all systems)
find_package(absl)
if(NOT absl_FOUND)
    message(STATUS "absl not found, continuing without it")
endif()

add_subdirectory(src/common)
add_subdirectory(src/proto)
add_subdirectory(src/exporter)
add_subdirectory(src/server)
add_subdirectory(src/client)
add_subdirectory(tests)
