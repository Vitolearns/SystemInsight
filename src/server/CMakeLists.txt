add_library(system_insight_metrics_repository
    metrics_repository.cc
)

target_include_directories(system_insight_metrics_repository
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(system_insight_metrics_repository
    PUBLIC
    system_insight_proto
)

add_library(system_insight_server_lib
    server_app.cc
    metrics_service_impl.cc
)

target_include_directories(system_insight_server_lib
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(system_insight_server_lib
    PUBLIC
    system_insight_common_config
    system_insight_common_logging
    system_insight_metrics_repository
    system_insight_exporter
    system_insight_proto
    gRPC::grpc++
)

add_executable(system_insight_server main.cc)
target_link_libraries(system_insight_server
    PRIVATE
    system_insight_common_config
    system_insight_common_logging
    system_insight_server_lib
    gflags::gflags
)

