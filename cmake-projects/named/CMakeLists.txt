cmake_minimum_required(VERSION 3.10)
project(quasar_named CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Strict compiler flags
add_compile_options(-Wall -Wextra -Werror -pedantic)

# Find dependencies
find_package(PkgConfig REQUIRED)

# Try finding yaml-cpp
pkg_check_modules(YAML_CPP yaml-cpp)
if (NOT YAML_CPP_FOUND)
    set(YAML_CPP_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/yaml-cpp")
    if (EXISTS "${YAML_CPP_LOCAL_DIR}/CMakeLists.txt")
        message(STATUS "Using local yaml-cpp from ${YAML_CPP_LOCAL_DIR}")
        set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
        add_subdirectory("${YAML_CPP_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/yaml-cpp")
        set(YAML_CPP_LIBRARIES yaml-cpp)
        set(YAML_CPP_INCLUDE_DIRS "${YAML_CPP_LOCAL_DIR}/include")
    else()
        message(STATUS "yaml-cpp not found via PkgConfig or local third-party, trying FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
          yaml-cpp
          GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git
          GIT_TAG 0.8.0
        )
        set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(yaml-cpp)
        set(YAML_CPP_LIBRARIES yaml-cpp)
        set(YAML_CPP_INCLUDE_DIRS ${yaml-cpp_SOURCE_DIR}/include)
    endif()
endif()

# Try finding tinyxml2
pkg_check_modules(TINYXML2 tinyxml2)
if (NOT TINYXML2_FOUND)
    set(TINYXML2_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/tinyxml2")
    if (EXISTS "${TINYXML2_LOCAL_DIR}/CMakeLists.txt")
        message(STATUS "Using local tinyxml2 from ${TINYXML2_LOCAL_DIR}")
        set(tinyxml2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
        add_subdirectory("${TINYXML2_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/tinyxml2")
        set(TINYXML2_LIBRARIES tinyxml2)
        set(TINYXML2_INCLUDE_DIRS "${TINYXML2_LOCAL_DIR}")
    else()
        message(STATUS "tinyxml2 not found via PkgConfig or local third-party, trying FetchContent...")
        include(FetchContent)
        FetchContent_Declare(
          tinyxml2
          GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
          GIT_TAG 10.0.0
        )
        FetchContent_MakeAvailable(tinyxml2)
        set(TINYXML2_LIBRARIES tinyxml2)
        set(TINYXML2_INCLUDE_DIRS ${tinyxml2_SOURCE_DIR})
    endif()
endif()

# Find sources
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp" "include/*.h")

add_library(quasar_named SHARED ${SOURCES} ${HEADERS})

target_include_directories(quasar_named PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    ${YAML_CPP_INCLUDE_DIRS}
    ${TINYXML2_INCLUDE_DIRS}
)

target_link_libraries(quasar_named PUBLIC
    quasar_coretypes
    ${YAML_CPP_LIBRARIES}
    ${TINYXML2_LIBRARIES}
)

# Testing
enable_testing()
add_subdirectory(test)
