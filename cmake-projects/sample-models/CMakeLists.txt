add_library(sample-models SHARED
    src/InverterModel.cpp
    src/SmpModel.cpp
    src/Library.cpp
)

target_include_directories(sample-models PRIVATE
    include
)

target_link_libraries(sample-models PRIVATE ECSS_SMP)
