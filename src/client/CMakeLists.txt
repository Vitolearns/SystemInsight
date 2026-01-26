add_library(system_insight_client_lib
    client_app.cc
    metrics_client.cc
    system_metrics_collector.cc
    metrics/mmap_reader.cc
    metrics/cpu_mmap_collector.cc
)

target_include_directories(system_insight_client_lib
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(system_insight_client_lib
    PUBLIC
    system_insight_common_config
    system_insight_common_logging
    system_insight_proto
    grpc::grpc++
)

add_executable(system_insight_client main.cc)
target_link_libraries(system_insight_client
    PRIVATE
    system_insight_client_lib
    gflags::gflags
)
