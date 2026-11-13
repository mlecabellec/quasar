cmake_minimum_required(VERSION 3.16)

if(NOT TARGET gtest)
    set(GTEST_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/googletest")
    if(EXISTS "${GTEST_LOCAL_DIR}/CMakeLists.txt")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory("${GTEST_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/googletest")
    endif()
endif()

add_executable(coretypes_extended_test TestCoretypesExtended.cpp)

target_link_libraries(coretypes_extended_test PRIVATE
    quasar_coretypes
    quasar_test_support
    GTest::gtest_main
)

target_compile_options(coretypes_extended_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(coretypes_extended_test)
