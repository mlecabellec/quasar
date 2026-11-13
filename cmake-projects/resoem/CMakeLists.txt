cmake_minimum_required(VERSION 3.25)

project(resoem LANGUAGES CXX)

# library
add_library(resoem STATIC
    src/RawSocket.cpp
    src/EtherCATFrame.cpp
    src/Enumerator.cpp
    src/MailboxHandler.cpp
    src/CoEHandler.cpp
    src/FoEHandler.cpp
    src/SoEHandler.cpp
    src/EoEHandler.cpp
    src/Diagnostics.cpp
    src/EthercatMasterService.cpp
    src/EthercatSlave.cpp
    src/EniDocument.cpp
    src/EniConfigurator.cpp
    src/virtual/SimulatedSlave.cpp
    src/virtual/VirtualEtherCATBus.cpp
    src/virtual/StubRawSocket.cpp
    src/RedundancyManager.cpp
    src/HotConnectManager.cpp
    src/EniGenerator.cpp
    src/EniComparator.cpp
)
quasar_apply_standards(resoem)
# ...
# Test Executable
add_executable(test_broadcast_read test/test_broadcast_read.cpp)
target_link_libraries(test_broadcast_read PRIVATE resoem)

add_executable(test_enumeration test/test_enumeration.cpp)
target_link_libraries(test_enumeration PRIVATE resoem)

add_executable(test_coe_upload test/test_coe_upload.cpp)
target_link_libraries(test_coe_upload PRIVATE resoem)

add_executable(test_soe_framing test/test_soe_framing.cpp)
target_link_libraries(test_soe_framing PRIVATE resoem)

add_executable(test_topology test/test_topology.cpp)
target_link_libraries(test_topology PRIVATE resoem)

add_executable(test_dc_calc test/test_dc_calc.cpp)
target_link_libraries(test_dc_calc PRIVATE resoem)

add_executable(test_redundancy test/test_redundancy.cpp)
target_link_libraries(test_redundancy PRIVATE resoem)

add_executable(test_diagnostics test/test_diagnostics.cpp)
target_link_libraries(test_diagnostics PRIVATE resoem)

add_executable(test_eni_parser test/test_eni_parser.cpp)
target_link_libraries(test_eni_parser PRIVATE resoem tinyxml2)

add_executable(test_virtual_bus test/test_virtual_bus.cpp)
target_link_libraries(test_virtual_bus PRIVATE resoem tinyxml2)

add_executable(test_eni_configurator test/test_eni_configurator.cpp)
target_link_libraries(test_eni_configurator PRIVATE resoem tinyxml2)

add_executable(test_redundancy_loop test/test_redundancy_loop.cpp)
target_link_libraries(test_redundancy_loop PRIVATE resoem tinyxml2)

add_executable(test_hot_connect test/test_hot_connect.cpp)
target_link_libraries(test_hot_connect PRIVATE resoem tinyxml2)

add_executable(test_eni_generator_comparator test/test_eni_generator_comparator.cpp)
target_link_libraries(test_eni_generator_comparator PRIVATE resoem tinyxml2)

add_executable(test_compliance_and_stress test/test_compliance_and_stress.cpp)
target_link_libraries(test_compliance_and_stress PRIVATE resoem)

# Samples
add_subdirectory(samples)

target_include_directories(resoem PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../third-party/tinyxml2>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(resoem PUBLIC cxx_std_23)
set_target_properties(resoem PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(resoem PUBLIC quasar_named utils tinyxml2)
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(Threads REQUIRED)
    target_link_libraries(resoem PRIVATE Threads::Threads)
endif()

# Tools
add_subdirectory(tools)

# Installation rules (if needed later)
