# Common source files for SMP models
set(COMMON_SRCS
    src/common/SmpModel.cpp
)

# Inverter Model Library
add_library(sample-inverter SHARED
    src/InverterModel.cpp
    src/InverterLib.cpp
    ${COMMON_SRCS}
)
target_include_directories(sample-inverter PRIVATE include)
target_link_libraries(sample-inverter PRIVATE ECSS_SMP)

# Delayed Simple Model Library
add_library(sample-delayed-simple SHARED
    src/DelayedModels.cpp
    src/DelayedSimpleLib.cpp
    ${COMMON_SRCS}
)
target_include_directories(sample-delayed-simple PRIVATE include)
target_link_libraries(sample-delayed-simple PRIVATE ECSS_SMP)
set_target_properties(sample-delayed-simple PROPERTIES COMPILE_DEFINITIONS "BUILD_DELAYED_SIMPLE")

# Delayed Array Model Library
add_library(sample-delayed-array SHARED
    src/DelayedModels.cpp
    src/DelayedArrayLib.cpp
    ${COMMON_SRCS}
)
target_include_directories(sample-delayed-array PRIVATE include)
target_link_libraries(sample-delayed-array PRIVATE ECSS_SMP)
set_target_properties(sample-delayed-array PROPERTIES COMPILE_DEFINITIONS "BUILD_DELAYED_ARRAY")

# Unit Tests for sample-models
add_executable(sample_models_test
    test/TestSampleModels.cpp
    src/InverterModel.cpp
    src/DelayedModels.cpp
    ${COMMON_SRCS}
)
target_include_directories(sample_models_test PRIVATE include)
target_link_libraries(sample_models_test PRIVATE
    ECSS_SMP
    sim
    quasar_test_support
    GTest::gtest_main
)
include(GoogleTest)
gtest_discover_tests(sample_models_test)
