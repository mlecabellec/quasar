cmake_minimum_required(VERSION 3.10)
project(quasar)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found - local build speedups disabled")
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

include(cmake-projects/QuasarStandards.cmake)

# Treat all third-party includes as SYSTEM globally
include_directories(SYSTEM cmake-projects/third-party)

# Options
option(QUASAR_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(QUASAR_ENABLE_TSAN "Enable ThreadSanitizer" OFF)

enable_testing()

add_subdirectory(cmake-projects)
option(BUILD_LINUX_MODULES "Build Linux kernel modules" ON)
if(BUILD_LINUX_MODULES)
    add_subdirectory(linux-modules)
endif()
