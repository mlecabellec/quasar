cmake_minimum_required(VERSION 3.10)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(QUASAR_ENABLE_RAYLIB "Enable Raylib" ON)

if(QUASAR_ENABLE_RAYLIB)
    # Configure Raylib
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
    add_subdirectory(raylib)
endif()

# Configure SymEngine
set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
set(WITH_SYMENGINE_THREAD_SAFE ON CACHE BOOL "" FORCE)
set(INTEGER_CLASS "gmp" CACHE STRING "" FORCE)
add_subdirectory(symengine)

# Other libraries
# Check if they are already added to avoid errors if this is included multiple times or in weird order
if(NOT TARGET gtest)
    add_subdirectory(googletest)
endif()

if(NOT TARGET tinyxml2)
    set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    add_subdirectory(tinyxml2)
endif()

if(NOT TARGET yaml-cpp)
    set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
    add_subdirectory(yaml-cpp)
endif()

# Verifications
add_subdirectory(symengine-verification)

# Note: raylib-verification is missing
