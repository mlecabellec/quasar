cmake_minimum_required(VERSION 3.10)
project(symengine_verification)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configure SymEngine
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(WITH_SYMENGINE_THREAD_SAFE ON CACHE BOOL "" FORCE)
set(INTEGER_CLASS "gmp" CACHE STRING "" FORCE) # Defaulting to GMP, assuming it's available

# add_subdirectory(../symengine symengine_build)

add_executable(symengine_test main.cpp)
target_include_directories(symengine_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../symengine
    ${CMAKE_CURRENT_BINARY_DIR}/../symengine
)
target_link_libraries(symengine_test PRIVATE symengine)

add_test(NAME symengine_test COMMAND symengine_test)
