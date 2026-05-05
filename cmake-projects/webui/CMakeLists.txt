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
    ${CMAKE_CURRENT_SOURCE_DIR}/../scripting/plugins/net/include
)
# --- Frontend Build Integration ---

quasar_add_svelte_project(
    TARGET_NAME quasar-web-resources
    PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/frontend-svelte"
    NAMESPACE quasar_web
)

find_program(NPM_EXECUTABLE npm)

if(NPM_EXECUTABLE)
    # [CS-0030.1] Feature: Unified Build Chain
    # Define a custom target to manage the React/TS build process.
    add_custom_target(quasar_webui_frontend
        COMMAND ${NPM_EXECUTABLE} install
        COMMAND ${NPM_EXECUTABLE} run build
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/frontend
        COMMENT "Building Mission Control Frontend (Vite/React)..."
        VERBATIM
    )

    # Ensure the C++ service depends on the frontend assets.
    add_dependencies(${PROJECT_NAME} quasar_webui_frontend)
else()
    message(WARNING "npm not found. Frontend build will be skipped.")
endif()

# --- Library Configuration ---
target_link_libraries(${PROJECT_NAME} PUBLIC
    quasar_named
    datacodec
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    cppserver
    cppcommon
    fmt
    quasar::quasar-web-resources
)

install(TARGETS ${PROJECT_NAME}
    LIBRARY DESTINATION lib
)

# --- Sample Application ---
add_executable(sampleweb001 src/sampleweb001.cpp)
target_include_directories(sampleweb001 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../logicengine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../opcua/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../scripting/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../coretypes/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/jsoncons/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppServer/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/CppCommon/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/fmt/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/asio/asio/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/sol2/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third-party/lua
)
target_link_libraries(sampleweb001 PRIVATE
    ${PROJECT_NAME}
    quasar_logicengine
    quasar_opcua
    quasar_named
    quasar_coretypes
    quasar::quasar-web-resources
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