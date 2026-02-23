add_executable(sim-test src/main.cpp)

target_include_directories(sim-test PRIVATE
    ${PROJECT_SOURCE_DIR}/smp/include
    ${PROJECT_SOURCE_DIR}/sim/include
    ${PROJECT_SOURCE_DIR}/third-party/core/include
    ${PROJECT_SOURCE_DIR}/sample-models/include
)

target_link_libraries(sim-test PRIVATE ECSS_SMP sim sample-models)

# New dynamic test executable
add_executable(dynamic-sim-test src/dynamic_main.cpp)

target_include_directories(dynamic-sim-test PRIVATE
    ${PROJECT_SOURCE_DIR}/smp/include
    ${PROJECT_SOURCE_DIR}/sim/include
    ${PROJECT_SOURCE_DIR}/third-party/core/include
)

# Note: We do NOT link against sample-models to ensure dynamic loading works
target_link_libraries(dynamic-sim-test PRIVATE ECSS_SMP sim)
