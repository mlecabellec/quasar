cmake_minimum_required(VERSION 3.25)

project(resoes LANGUAGES CXX)

add_library(resoes STATIC
    src/common.cpp
    src/hal/MemoryEscHardware.cpp
    src/EepromEmulator.cpp
    src/EscController.cpp
    src/MailboxServer.cpp
    src/ObjectDictionary.cpp
    src/CoeServer.cpp
    src/SlaveApplication.cpp
)

quasar_apply_standards(resoes)

target_include_directories(resoes PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(resoes PUBLIC cxx_std_23)
set_target_properties(resoes PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(Threads REQUIRED)
    target_link_libraries(resoes PRIVATE Threads::Threads)
endif()

# Unit tests
add_executable(test_resoes_esm_eeprom test/test_esm_and_eeprom.cpp)
quasar_apply_standards(test_resoes_esm_eeprom)
target_link_libraries(test_resoes_esm_eeprom PRIVATE resoes)
add_test(NAME resoes_esm_eeprom COMMAND test_resoes_esm_eeprom)

add_executable(test_resoes_coe_pdo test/test_coe_and_pdo.cpp)
quasar_apply_standards(test_resoes_coe_pdo)
target_link_libraries(test_resoes_coe_pdo PRIVATE resoes)
add_test(NAME resoes_coe_pdo COMMAND test_resoes_coe_pdo)
