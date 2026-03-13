add_executable(sim-test src/main.cpp)
target_include_directories(sim-test PRIVATE
    ${PROJECT_SOURCE_DIR}/smp/include
    ${PROJECT_SOURCE_DIR}/sim/include
    ${PROJECT_SOURCE_DIR}/third-party/core/include
    ${PROJECT_SOURCE_DIR}/sample-models/include
)
target_link_libraries(sim-test PRIVATE ECSS_SMP sim sample-inverter)

# New dynamic test executable
add_executable(dynamic-sim-test src/dynamic_main.cpp)
target_include_directories(dynamic-sim-test PRIVATE
    ${PROJECT_SOURCE_DIR}/smp/include
    ${PROJECT_SOURCE_DIR}/sim/include
    ${PROJECT_SOURCE_DIR}/third-party/core/include
)
target_link_libraries(dynamic-sim-test PRIVATE ECSS_SMP sim)

# Shared include directories for all test executables
set(TEST_INCLUDE_DIRS
    ${PROJECT_SOURCE_DIR}/smp/include
    ${PROJECT_SOURCE_DIR}/sim/include
    ${PROJECT_SOURCE_DIR}/third-party/core/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# test-inverter
add_executable(test-inverter src/test_inverter.cpp)
target_include_directories(test-inverter PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(test-inverter PRIVATE ECSS_SMP sim)

# test-delayed-simple
add_executable(test-delayed-simple src/test_delayed_simple.cpp)
target_include_directories(test-delayed-simple PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(test-delayed-simple PRIVATE ECSS_SMP sim)

# test-delayed-array
add_executable(test-delayed-array src/test_delayed_array.cpp)
target_include_directories(test-delayed-array PRIVATE ${TEST_INCLUDE_DIRS})
target_link_libraries(test-delayed-array PRIVATE ECSS_SMP sim)

# CTest definitions
add_test(NAME sim-test COMMAND sim-test)
add_test(NAME dynamic-sim-test COMMAND dynamic-sim-test)
add_test(NAME test-inverter COMMAND test-inverter)
add_test(NAME test-delayed-simple COMMAND test-delayed-simple)
add_test(NAME test-delayed-array COMMAND test-delayed-array)
