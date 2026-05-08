cmake_minimum_required(VERSION 3.10)
project(quasar_logicengine)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(include)
include_directories(../coretypes/include)
include_directories(../named/include)

include_directories(../scripting/include)

# Source files
set(SOURCE_FILES
    src/BitVector.cpp
    src/LogicComponent.cpp
    src/LogicEngine.cpp
    src/CauseEffectMatrix.cpp
    src/StateMachine.cpp
    src/SFC.cpp
    src/Expression.cpp
    src/EvaluationPool.cpp
)

# Library target
add_library(quasar_logicengine STATIC ${SOURCE_FILES})

quasar_apply_standards(quasar_logicengine)

# Link dependencies
target_link_libraries(quasar_logicengine quasar_coretypes quasar_named quasar_scripting pthread)

# Testing
if(BUILD_TESTING)
    if(NOT TARGET gtest)
        set(GTEST_LOCAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../third-party/googletest")
        if(EXISTS "${GTEST_LOCAL_DIR}/CMakeLists.txt")
            message(STATUS "Using local googletest from ${GTEST_LOCAL_DIR}")
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            add_subdirectory("${GTEST_LOCAL_DIR}" "${CMAKE_BINARY_DIR}/third-party/googletest")
        else()
            include(FetchContent)
            FetchContent_Declare(
              googletest
              URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
            )
            set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(googletest)
        endif()
    endif()

    add_executable(logicengine_test 
        test/LogicEngineTest.cpp
        test/StateMachineTest.cpp
        test/SFCTest.cpp
        test/LogicStressTest.cpp
    )
    target_link_libraries(logicengine_test quasar_logicengine GTest::gtest_main)
    
    include(GoogleTest)
    gtest_discover_tests(logicengine_test)
endif()
