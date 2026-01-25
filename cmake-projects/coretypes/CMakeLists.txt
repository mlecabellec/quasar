cmake_minimum_required(VERSION 3.10)
project(quasar_coretypes CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Strict compiler flags
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Find sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_library(quasar_coretypes SHARED ${SOURCES} ${HEADERS})

target_include_directories(quasar_coretypes PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Testing
enable_testing()
add_subdirectory(test)
