cmake_minimum_required(VERSION 3.10)
project(quasar_calibration)

add_library(quasar_calibration
    src/Calibration.cpp
    src/LuaBindings.cpp
)

target_include_directories(quasar_calibration PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../third-party/lua>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../third-party/sol2/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(quasar_calibration PUBLIC
    quasar_coretypes
    quasar_named
    sol2
    liblua
)

target_compile_features(quasar_calibration PUBLIC cxx_std_23)

# Test executable
add_executable(quasar_calibration_test test/main.cpp test/TestCalibrations.cpp)
target_link_libraries(quasar_calibration_test PRIVATE quasar_calibration GTest::gtest_main)
add_test(NAME quasar_calibration_test COMMAND quasar_calibration_test)
