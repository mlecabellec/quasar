project(quasar_webui)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
    src/WebUIService.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppServer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppServer/include/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppCommon/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/jsoncons/include
)

target_link_libraries(${PROJECT_NAME} PUBLIC
    quasar_named
    datacodec
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cppserver
    cppcommon
    fmt
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
)

add_executable(webui_test test/TestWebUI.cpp)
target_include_directories(webui_test PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppServer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppServer/include/server
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppCommon/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/jsoncons/include
)
target_link_libraries(webui_test PRIVATE
    ${PROJECT_NAME}
    GTest::gtest_main
    Catch2::Catch2WithMain
)

include(CTest)
add_test(NAME webui_test COMMAND webui_test)