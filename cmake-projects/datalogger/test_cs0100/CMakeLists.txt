add_executable(datalogger_extended_test TestDataloggerExtended.cpp)
target_link_libraries(datalogger_extended_test PRIVATE
    quasar_datalogger
    quasar_named
    quasar_coretypes
    quasar_test_support
    GTest::gtest_main
)
target_compile_options(datalogger_extended_test PRIVATE -Wno-error=restrict)
include(GoogleTest)
gtest_discover_tests(datalogger_extended_test)
