cmake_minimum_required(VERSION 3.14)
project(datacodec)

# Use C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)

# Try finding jsoncons
pkg_check_modules(JSONCONS jsoncons)
if (NOT JSONCONS_FOUND AND NOT TARGET jsoncons)
    set(JSONCONS_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/jsoncons")
    if (EXISTS "${JSONCONS_LOCAL_DIR}/CMakeLists.txt")
        message(STATUS "Using local jsoncons from ${JSONCONS_LOCAL_DIR}")
        add_subdirectory("${JSONCONS_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/jsoncons")
        set(JSONCONS_LIBRARIES jsoncons)
        set(JSONCONS_INCLUDE_DIRS "${JSONCONS_LOCAL_DIR}/include")
    else()
        message(STATUS "jsoncons not found via PkgConfig or local third-party, trying FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
          jsoncons
          GIT_REPOSITORY https://github.com/danielaparker/jsoncons.git
          GIT_TAG v0.173.0
        )
        set(JSONCONS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(jsoncons)
        set(JSONCONS_LIBRARIES jsoncons)
        set(JSONCONS_INCLUDE_DIRS ${jsoncons_SOURCE_DIR}/include)
    endif()
endif()

# Define the library
add_library(datacodec
    src/ICodec.cpp 
)

# Setup include directories
target_include_directories(datacodec PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/../coretypes/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../named/include
    ${JSONCONS_INCLUDE_DIRS}
)

# Link dependencies
target_link_libraries(datacodec PUBLIC
    quasar_coretypes
    quasar_named
    ${JSONCONS_LIBRARIES}
)

quasar_apply_standards(datacodec)

# Testing
enable_testing()
add_subdirectory(test)
