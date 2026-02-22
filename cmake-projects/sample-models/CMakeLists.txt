add_library(sample-models SHARED
    src/InverterModel.cpp
    src/Library.cpp
)

target_include_directories(sample-models PRIVATE
    include
    ${PROJECT_SOURCE_DIR}/smp/include
    ${PROJECT_SOURCE_DIR}/sim/include
    ${PROJECT_SOURCE_DIR}/third_party/core/include
)

target_link_libraries(sample-models PRIVATE smp sim)
