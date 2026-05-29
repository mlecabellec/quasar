project(quasar_net_plugin)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
    src/NetPlugin.cpp
    src/EventTrampoline.cpp
    src/TCPServerWrapper.cpp
    src/UDPServerWrapper.cpp
    src/WS_Server.cpp
    src/WSS_Server.cpp
    src/TCPClientWrapper.cpp
    src/UDPClientWrapper.cpp
    src/HTTPClientWrapper.cpp
    src/WSClientWrapper.cpp
    src/SSLContextWrapper.cpp
    src/WebNamedMethodBindings.cpp
    src/RawEthernetServiceBindings.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Need ASIO, CppServer, CppCommon, fmt, OpenSSL, and quasar_scripting
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppServer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppServer/include/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppServer/include/client
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppCommon/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Dependencies
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})

# Install
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
)

# Test Sandbox
add_executable(quasar_net_sandbox test/TestSandbox.cpp)
target_include_directories(quasar_net_sandbox PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppServer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppServer/include/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppServer/include/client
)

target_link_libraries(quasar_net_sandbox PRIVATE
    ${PROJECT_NAME}
    quasar_scripting
    cppserver
    cppcommon
    fmt
    sol2
    liblua
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    quasar_scripting
    cppserver
    cppcommon
    fmt
    sol2
    liblua
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
)

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
