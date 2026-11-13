cmake_minimum_required(VERSION 3.16)

add_executable(integration_smp_lua_opcua_test TestIntegrationSmpLuaOpcua.cpp)
target_link_libraries(integration_smp_lua_opcua_test PRIVATE
    sim
    ECSS_SMP
    core
    utils
    sched
    quasar_scripting
    quasar_opcua
    quasar_named
    quasar_coretypes
    quasar_test_support
    open62541
    GTest::gtest_main
)
target_compile_options(integration_smp_lua_opcua_test PRIVATE -Wno-error=restrict)

add_executable(integration_resoem_resoes_test TestIntegrationResoemResoes.cpp)
target_link_libraries(integration_resoem_resoes_test PRIVATE
    resoem
    resoes
    quasar_named
    quasar_coretypes
    quasar_test_support
    tinyxml2
    GTest::gtest_main
)
target_compile_options(integration_resoem_resoes_test PRIVATE -Wno-error=restrict)

add_executable(integration_multi_plugin_test TestIntegrationMultiPlugin.cpp)
target_include_directories(integration_multi_plugin_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/libzmq/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../logicengine/include
)
target_link_libraries(integration_multi_plugin_test PRIVATE
    quasar_scripting
    quasar_opcua
    quasar_logicengine
    quasar_calibration
    quasar_datalogger
    datacodec
    quasar_named
    quasar_coretypes
    quasar_test_support
    open62541
    libzmq
    GTest::gtest_main
)
target_compile_options(integration_multi_plugin_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(integration_smp_lua_opcua_test)
gtest_discover_tests(integration_resoem_resoes_test)
gtest_discover_tests(integration_multi_plugin_test)
