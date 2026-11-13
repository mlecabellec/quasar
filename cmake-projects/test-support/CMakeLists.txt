cmake_minimum_required(VERSION 3.14)
project(quasar_test_support LANGUAGES CXX)

find_package(Threads REQUIRED)

if(NOT TARGET gtest)
    set(GTEST_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/googletest")
    if(EXISTS "${GTEST_LOCAL_DIR}/CMakeLists.txt")
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        add_subdirectory("${GTEST_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/googletest")
    endif()
endif()

add_library(quasar_test_support INTERFACE)
target_include_directories(quasar_test_support INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_compile_features(quasar_test_support INTERFACE cxx_std_23)
target_link_libraries(quasar_test_support INTERFACE Threads::Threads)
