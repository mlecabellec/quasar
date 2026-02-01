cmake_minimum_required(VERSION 3.10)

if(NOT TARGET gtest)
    set(GTEST_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/googletest")
    if(EXISTS "${GTEST_LOCAL_DIR}/CMakeLists.txt")
        message(STATUS "Using local googletest from ${GTEST_LOCAL_DIR}")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory("${GTEST_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/googletest")
    else()
        message(STATUS "googletest not found in local third-party, trying FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
          googletest
          URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
    endif()
endif()

# Force re-glob
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_executable(named_test ${TEST_SOURCES})

target_link_libraries(named_test PRIVATE
    quasar_named
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(named_test)