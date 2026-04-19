cmake_minimum_required(VERSION 3.10)
project(quasar_manual_tests)

if(NOT TARGET gtest)
    set(GTEST_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/googletest")
    if(EXISTS "${GTEST_LOCAL_DIR}/CMakeLists.txt")
        add_subdirectory("${GTEST_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/googletest")
    endif()
endif()

add_executable(manual_tests TestManualScripts.cpp)

target_link_libraries(manual_tests PRIVATE 
    quasar_scripting 
    GTest::gtest_main
)

# Ensure all plugins are built before the tests
add_dependencies(manual_tests 
    quasar_zmq 
    quasar_net_plugin 
    quasar_opcua_plugin 
    quasar_datalogger_plugin
)

# Get the directory where plugins are built
# In our project structure, they are usually in ${CMAKE_BINARY_DIR}/lib
target_compile_definitions(manual_tests PRIVATE 
    QUASAR_MANUAL_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../../doc/manual"
    QUASAR_PLUGIN_DIR="${CMAKE_BINARY_DIR}/lib"
)

include(GoogleTest)
gtest_discover_tests(manual_tests)
