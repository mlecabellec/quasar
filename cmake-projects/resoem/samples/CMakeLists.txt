cmake_minimum_required(VERSION 3.10)

find_package(Python3 REQUIRED)

# Include the AddENI macro
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/AddENI.cmake)

# Link against resoem
# Assuming resoem target is available from parent project

# 1. simple_ng
add_executable(simple_ng simple_ng.cpp)
target_link_libraries(simple_ng PRIVATE resoem)

# 2. slaveinfo
add_executable(slaveinfo slaveinfo.cpp)
target_link_libraries(slaveinfo PRIVATE resoem)

# 3. eepromtool
add_executable(eepromtool eepromtool.cpp)
target_link_libraries(eepromtool PRIVATE resoem)

# 4. firm_update
add_executable(firm_update firm_update.cpp)
target_link_libraries(firm_update PRIVATE resoem)

# 5. eoe_test
add_executable(eoe_test eoe_test.cpp)
target_link_libraries(eoe_test PRIVATE resoem)

# 6. eni_test
add_executable(eni_test eni_test.cpp)
# Ensure sample-eni.xml exists in the current directory or provide path
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/sample-eni.xml")
    AddENI(eni_test sample-eni.xml)
endif()
target_link_libraries(eni_test PRIVATE resoem)
