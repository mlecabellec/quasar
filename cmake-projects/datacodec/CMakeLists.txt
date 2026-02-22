cmake_minimum_required(VERSION 3.14)
project(datacodec)

# Use C++23 standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies
find_package(coretypes REQUIRED)
find_package(named REQUIRED)

# Define the library
add_library(datacodec
    src/ICodec.cpp # Dummy file for now if header-only or add real sources later
)

# Setup include directories
target_include_directories(datacodec PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Link dependencies
target_link_libraries(datacodec PUBLIC
    coretypes::coretypes
    named::named
)
