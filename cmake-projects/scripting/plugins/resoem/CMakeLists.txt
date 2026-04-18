cmake_minimum_required(VERSION 3.25)
project(quasar_resoem_plugin CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Source files
set(SOURCES
    src/ResoemPlugin.cpp
)

# Create the library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/sol2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/lua
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../coretypes/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../named/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../resoem/include
)

# Link dependencies
target_link_libraries(${PROJECT_NAME} PRIVATE
    quasar_scripting
    quasar_named
    resoem
    sol2
    liblua
)

# Installation
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)
