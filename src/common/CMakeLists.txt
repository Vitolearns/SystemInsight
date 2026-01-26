add_library(system_insight_common_logging
    logging/logging.cc
)

target_include_directories(system_insight_common_logging
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(system_insight_common_logging
    PUBLIC
    spdlog::spdlog
)

add_library(system_insight_common_config
    config/config_loader.cc
)

target_include_directories(system_insight_common_config
    PUBLIC
    ${PROJECT_SOURCE_DIR}
)

target_link_libraries(system_insight_common_config
    PUBLIC
    system_insight_common_logging
    nlohmann_json::nlohmann_json
)
