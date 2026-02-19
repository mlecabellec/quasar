add_library(ECSS_SMP INTERFACE)

target_include_directories(ECSS_SMP INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
