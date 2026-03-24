project(quasar_net_plugin)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
    src/NetPlugin.cpp
    src/TCPServerWrapper.cpp
    src/UDPServerWrapper.cpp
    src/WebServerWrapper.cpp
    src/TCPClientWrapper.cpp
    src/HTTPClientWrapper.cpp
    src/SSLContextWrapper.cpp
)

set(HEADERS
    # Will add headers here
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Need ASIO, CppServer, CppCommon, fmt, OpenSSL, and quasar_scripting
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppServer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/CppCommon/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../third-party/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# OpenSSL from# Dependencies
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIR})

# Install
install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
)

# Test Sandbox
add_executable(quasar_net_sandbox test/TestSandbox.cpp)
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

# Plugin expects to be dynamically loaded without a known name by the OS, 
# although we compile it as SHARED.
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
