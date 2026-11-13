cmake_minimum_required(VERSION 3.16)

add_executable(resoem_extended_test TestResoemExtended.cpp)

target_link_libraries(resoem_extended_test PRIVATE
    resoem
    quasar_test_support
    tinyxml2
    GTest::gtest_main
)

target_compile_options(resoem_extended_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(resoem_extended_test)
