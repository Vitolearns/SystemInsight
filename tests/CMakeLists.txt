enable_testing()

find_package(PkgConfig)
pkg_check_modules(GTEST gtest)

if(GTEST_FOUND)
    add_executable(config_loader_test config_loader_test.cc)

    target_include_directories(config_loader_test PRIVATE ${GTEST_INCLUDE_DIRS})
    target_link_libraries(config_loader_test PRIVATE
        system_insight_common_config
        ${GTEST_LIBRARIES}
        gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(config_loader_test)
else()
    message(STATUS "GTest not found, skipping tests")
endif()
