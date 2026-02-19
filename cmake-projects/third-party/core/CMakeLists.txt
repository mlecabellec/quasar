add_library(core INTERFACE)

target_include_directories(core INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(core INTERFACE ECSS_SMP)
