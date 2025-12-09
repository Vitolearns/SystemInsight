add_library(system_insight_exporter
    prometheus_exporter.cc
)

target_include_directories(system_insight_exporter
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(system_insight_exporter
    PUBLIC
    system_insight_common_logging
    system_insight_metrics_repository
)

