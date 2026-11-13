cmake_minimum_required(VERSION 3.16)

add_executable(logicengine_extended_test TestLogicEngineExtended.cpp)

target_link_libraries(logicengine_extended_test PRIVATE
    quasar_logicengine
    quasar_test_support
    GTest::gtest_main
)

target_compile_options(logicengine_extended_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(logicengine_extended_test)
