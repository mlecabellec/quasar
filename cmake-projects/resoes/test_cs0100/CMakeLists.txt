cmake_minimum_required(VERSION 3.16)

add_executable(resoes_extended_test TestResoesExtended.cpp)

target_link_libraries(resoes_extended_test PRIVATE
    resoes
    quasar_test_support
    GTest::gtest_main
)

target_compile_options(resoes_extended_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(resoes_extended_test)
