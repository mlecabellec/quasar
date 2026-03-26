cmake_minimum_required(VERSION 3.10)
project(quasar_zmq CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if libzmq target is already available
if (NOT TARGET libzmq)
    find_package(PkgConfig QUIET)
    if (PKG_CONFIG_FOUND)
        pkg_check_modules(ZMQ QUIET libzmq)
    endif()

    if (NOT ZMQ_FOUND)
        set(ZMQ_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/libzmq")
        if (EXISTS "${ZMQ_LOCAL_DIR}/CMakeLists.txt")
            message(STATUS "Using local libzmq from ${ZMQ_LOCAL_DIR}")
            set(WITH_PERF_TOOL OFF CACHE BOOL "" FORCE)
            set(ZMQ_BUILD_TESTS OFF CACHE BOOL "" FORCE)
            set(ENABLE_CPACK OFF CACHE BOOL "" FORCE)
            add_subdirectory("${ZMQ_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/libzmq")
            set(ZMQ_LIBRARIES libzmq)
            set(ZMQ_INCLUDE_DIRS "${ZMQ_LOCAL_DIR}/include")
        endif()
    endif()
endif()

add_library(quasar_zmq SHARED src/ZmqBindings.cpp)

# Link to libzmq target if it exists, otherwise use variables from pkg-config
if (TARGET libzmq)
    target_link_libraries(quasar_zmq PRIVATE libzmq)
    target_include_directories(quasar_zmq PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/libzmq/include")
else()
    target_link_libraries(quasar_zmq PRIVATE ${ZMQ_LIBRARIES})
    target_include_directories(quasar_zmq PRIVATE ${ZMQ_INCLUDE_DIRS})
endif()

target_include_directories(quasar_zmq PRIVATE
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../named/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../coretypes/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/lua
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/sol2/include
)

target_link_libraries(quasar_zmq PRIVATE
    quasar_named
)

set_target_properties(quasar_zmq PROPERTIES PREFIX "")

if(BUILD_TESTING)
    add_executable(zmq_plugin_test test/TestZmqPlugin.cpp)
    
    target_include_directories(zmq_plugin_test PRIVATE
        include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../named/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../coretypes/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/lua
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/sol2/include
        ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/googletest/googletest/include
    )
    
    target_link_libraries(zmq_plugin_test PRIVATE
        gtest_main
        quasar_zmq
        quasar_named
    )
    
    if (TARGET libzmq)
        target_link_libraries(zmq_plugin_test PRIVATE libzmq)
    else()
        target_link_libraries(zmq_plugin_test PRIVATE ${ZMQ_LIBRARIES})
    endif()

    add_test(NAME ZmqPluginTest COMMAND zmq_plugin_test)
endif()
