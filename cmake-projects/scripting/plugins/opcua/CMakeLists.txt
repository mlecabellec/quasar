cmake_minimum_required(VERSION 3.10)
project(quasar_opcua_plugin CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(quasar_opcua_plugin SHARED src/OpcUaPlugin.cpp)

target_include_directories(quasar_opcua_plugin PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../coretypes/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../named/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../scripting/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../opcua/include
)

target_link_libraries(quasar_opcua_plugin PRIVATE
    quasar_opcua
    quasar_scripting
    quasar_named
    quasar_coretypes
)

set_target_properties(quasar_opcua_plugin PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
