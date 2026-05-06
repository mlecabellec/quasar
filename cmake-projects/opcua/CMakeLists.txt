cmake_minimum_required(VERSION 3.10)
project(quasar_opcua CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Find sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_library(quasar_opcua SHARED ${SOURCES} ${HEADERS})

target_compile_options(quasar_opcua PRIVATE -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-error=unused-parameter)

target_include_directories(quasar_opcua PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${CMAKE_CURRENT_SOURCE_DIR}/../coretypes/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../named/include
)

target_link_libraries(quasar_opcua PUBLIC
    quasar_named
    quasar_coretypes
    open62541
    utils
)

# Installation
install(TARGETS quasar_opcua
    EXPORT quasar_opcua_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

# Testing
enable_testing()
add_subdirectory(test)

