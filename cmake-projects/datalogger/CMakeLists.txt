cmake_minimum_required(VERSION 3.23)
project(quasar_datalogger CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

add_library(quasar_datalogger SHARED ${SOURCES} ${HEADERS})

target_compile_options(quasar_datalogger PRIVATE -Wall -Wextra -Werror -pedantic)

target_include_directories(quasar_datalogger PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(quasar_datalogger PUBLIC quasar_named quasar_coretypes)

install(TARGETS quasar_datalogger
    EXPORT quasar_datalogger_targets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/
    DESTINATION include
)

enable_testing()
add_subdirectory(test)
