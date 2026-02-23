add_executable(sim-test src/main.cpp)

target_include_directories(sim-test PRIVATE
    ${PROJECT_SOURCE_DIR}/smp/include
    ${PROJECT_SOURCE_DIR}/sim/include
    ${PROJECT_SOURCE_DIR}/third_party/core/include
    ${PROJECT_SOURCE_DIR}/sample-models/include
)

target_link_libraries(sim-test PRIVATE ECSS_SMP sim sample-models)
