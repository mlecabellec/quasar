add_library(sched
    src/Scheduler.cpp
)

target_include_directories(sched PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(sched PUBLIC smp utils)
