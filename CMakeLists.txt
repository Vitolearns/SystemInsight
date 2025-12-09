cmake_minimum_required(VERSION 3.20)

project(system_insight LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/third_party/cmake_cache")
file(MAKE_DIRECTORY "${FETCHCONTENT_BASE_DIR}")

include(FetchContent)

set(GFLAGS_USE_TARGET_NAMESPACE TRUE)

# Prefer config packages so that modern imported targets (and their deps like Abseil)
# are wired correctly, especially for protobuf / gRPC.
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG QUIET)
find_package(spdlog CONFIG QUIET)
find_package(gflags CONFIG QUIET)
find_package(nlohmann_json CONFIG QUIET)
find_package(GTest CONFIG QUIET)

# Abseil is required by newer protobuf versions for logging / checks; link via targets.
find_package(absl CONFIG REQUIRED)

if(NOT gRPC_FOUND)
    message(STATUS "gRPC CONFIG package not found, falling back to FetchContent.")
    FetchContent_Declare(
        grpc
        GIT_REPOSITORY https://github.com/grpc/grpc.git
        GIT_TAG v1.62.0
        GIT_PROGRESS TRUE
        GIT_SHALLOW FALSE
        GIT_SUBMODULES_RECURSE TRUE
    )
    set(FETCHCONTENT_QUIET OFF)
    FetchContent_MakeAvailable(grpc)
endif()

if(NOT spdlog_FOUND)
    message(STATUS "spdlog CONFIG package not found, falling back to FetchContent.")
FetchContent_Declare(
        spdlog
        URL https://github.com/gabime/spdlog/archive/refs/tags/v1.13.0.zip
)
    FetchContent_MakeAvailable(spdlog)
endif()

if(NOT gflags_FOUND)
    message(STATUS "gflags CONFIG package not found, falling back to FetchContent.")
FetchContent_Declare(
    gflags
    URL https://github.com/gflags/gflags/archive/refs/tags/v2.2.2.zip
)
FetchContent_MakeAvailable(gflags)
endif()

if(NOT nlohmann_json_FOUND)
    message(STATUS "nlohmann_json CONFIG package not found, falling back to FetchContent.")
FetchContent_Declare(
    nlohmann_json
    URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.zip
)
FetchContent_MakeAvailable(nlohmann_json)
endif()

if(NOT GTest_FOUND)
    message(STATUS "GTest CONFIG package not found, falling back to FetchContent.")
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    FetchContent_MakeAvailable(googletest)
endif()

add_subdirectory(src/common)
add_subdirectory(src/proto)
add_subdirectory(src/exporter)
add_subdirectory(src/server)
add_subdirectory(src/client)
add_subdirectory(tests)
