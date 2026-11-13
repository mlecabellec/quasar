cmake_minimum_required(VERSION 3.16)

add_executable(scripting_extended_test
    TestScriptingExtended.cpp
    ../qlsh/ShellUtilities.cpp
)

target_include_directories(scripting_extended_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../qlsh
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/ftxui/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/lua
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/sol2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../coretypes/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../named/include
)

target_link_libraries(scripting_extended_test PRIVATE
    quasar_scripting
    quasar_test_support
    screen
    dom
    component
    GTest::gtest_main
)

target_compile_options(scripting_extended_test PRIVATE -Wno-error=restrict)

include(GoogleTest)
gtest_discover_tests(scripting_extended_test)
