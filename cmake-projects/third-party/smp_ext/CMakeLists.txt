add_library(smp_ext INTERFACE)

target_include_directories(smp_ext INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(smp_ext INTERFACE ECSS_SMP)
