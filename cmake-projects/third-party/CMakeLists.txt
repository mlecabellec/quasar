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
option(QUASAR_ENABLE_SYMENGINE "Enable SymEngine" ON)

if(QUASAR_ENABLE_SYMENGINE)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    set(WITH_SYMENGINE_THREAD_SAFE ON CACHE BOOL "" FORCE)
    set(INTEGER_CLASS "gmp" CACHE STRING "" FORCE)
    add_subdirectory(symengine)
endif()

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

# Configure sol2
option(QUASAR_ENABLE_SOL2 "Enable sol2" ON)

if(QUASAR_ENABLE_SOL2)
    set(SOL2_TESTS ON CACHE BOOL "" FORCE)
    set(SOL2_LUA_VERSION "5.4.7" CACHE STRING "" FORCE)
    add_subdirectory(sol2)
endif()

# Configure Lua
option(QUASAR_ENABLE_LUA "Enable Lua" ON)

if(QUASAR_ENABLE_LUA)
    add_subdirectory(lua)
endif()

# Verifications
if(QUASAR_ENABLE_SYMENGINE)
    add_subdirectory(symengine-verification)
endif()

# Note: raylib-verification is missing

# Configure Catch2
option(QUASAR_ENABLE_CATCH2 "Enable Catch2" ON)

if(QUASAR_ENABLE_CATCH2)
    set(CATCH_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    set(CATCH_INSTALL_EXTRAS OFF CACHE BOOL "" FORCE)
    set(CATCH_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    set(CATCH_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(CATCH_BUILD_EXTRA_TESTS OFF CACHE BOOL "" FORCE)
    set(CATCH_BUILD_FUZZERS OFF CACHE BOOL "" FORCE)
    set(CATCH_BUILD_BENCHMARKS OFF CACHE BOOL "" FORCE)
    add_subdirectory(Catch2)
endif()

# Configure libzmq
option(QUASAR_ENABLE_LIBZMQ "Enable libzmq" ON)

if(QUASAR_ENABLE_LIBZMQ)
    set(BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(WITH_PERF_TOOL OFF CACHE BOOL "" FORCE)
    set(WITH_DOC OFF CACHE BOOL "" FORCE)
    set(ENABLE_DRAFTS OFF CACHE BOOL "" FORCE)
    set(WITH_LIBSODIUM OFF CACHE BOOL "" FORCE)
    set(WITH_GSSAPI_KRB5 OFF CACHE BOOL "" FORCE)
    add_subdirectory(libzmq)
endif()


