cmake_minimum_required(VERSION 3.10)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Force re-glob
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_executable(named_test ${TEST_SOURCES})

target_link_libraries(named_test PRIVATE
    quasar_named
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(named_test)