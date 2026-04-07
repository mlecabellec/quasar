cmake_minimum_required(VERSION 3.23)

if(NOT TARGET gtest)
    set(GTEST_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/googletest")
    if(EXISTS "${GTEST_LOCAL_DIR}/CMakeLists.txt")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory("${GTEST_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/googletest")
    endif()
endif()

file(GLOB_RECURSE TEST_SOURCES "*.cpp")

add_executable(datalogger_test ${TEST_SOURCES})

target_link_libraries(datalogger_test PRIVATE 
    quasar_datalogger 
    quasar_named
    quasar_coretypes
    GTest::gtest_main
)

target_compile_options(datalogger_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(datalogger_test)
