cmake_minimum_required(VERSION 3.25)

project(resoem LANGUAGES CXX)

# library
add_library(resoem STATIC
    src/RawSocket.cpp
    src/EtherCATFrame.cpp
    src/Enumerator.cpp
    src/MailboxHandler.cpp
    src/CoEHandler.cpp
)
# ...
# Test Executable
add_executable(test_broadcast_read test/test_broadcast_read.cpp)
target_link_libraries(test_broadcast_read PRIVATE resoem)

add_executable(test_enumeration test/test_enumeration.cpp)
target_link_libraries(test_enumeration PRIVATE resoem)

add_executable(test_coe_upload test/test_coe_upload.cpp)
target_link_libraries(test_coe_upload PRIVATE resoem)

target_include_directories(resoem PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(resoem PUBLIC cxx_std_23)

if(CMAKE_SYSTEM_NAME MATCHES "Linux")
    find_package(Threads REQUIRED)
    target_link_libraries(resoem PRIVATE Threads::Threads)
endif()




# Installation rules (if needed later)
