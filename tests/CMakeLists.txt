enable_testing()

add_executable(config_loader_test
    config_loader_test.cc
)

target_link_libraries(config_loader_test
    PRIVATE
    system_insight_common_config
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(config_loader_test)

