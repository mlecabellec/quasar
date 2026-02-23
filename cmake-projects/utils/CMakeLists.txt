add_library(utils
    src/Logger.cpp
    src/TimeKeeper.cpp
    src/EventManager.cpp
)

target_include_directories(utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(utils PUBLIC ECSS_SMP core)
