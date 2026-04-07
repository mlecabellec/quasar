# Test executable
add_executable(opcua_test
    TestOpcUaServer.cpp
    TestOpcUaClient.cpp
)

target_link_libraries(opcua_test PRIVATE
    quasar_opcua
    quasar_named
    quasar_coretypes
    open62541
    gtest
    gtest_main
)

add_test(NAME opcua_test COMMAND opcua_test)

