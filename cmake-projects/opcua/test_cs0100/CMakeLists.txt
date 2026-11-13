cmake_minimum_required(VERSION 3.16)

add_executable(opcua_extended_test TestOpcUaExtended.cpp)

target_link_libraries(opcua_extended_test PRIVATE
    quasar_opcua
    quasar_named
    quasar_coretypes
    quasar_test_support
    open62541
    GTest::gtest_main
)

target_compile_options(opcua_extended_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(opcua_extended_test)
