cmake_minimum_required(VERSION 3.10)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
# Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_executable(coretypes_test ${TEST_SOURCES})

target_link_libraries(coretypes_test PRIVATE 
    quasar_coretypes 
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(coretypes_test)
