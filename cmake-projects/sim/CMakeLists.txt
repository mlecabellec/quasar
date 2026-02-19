add_library(sim
    src/Simulator.cpp
    src/Resolver.cpp
    src/LinkRegistry.cpp
    src/Model.cpp
)

target_include_directories(sim PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(sim PUBLIC smp core utils sched)

# Test executable
add_executable(test_basic_simulation test/test_basic_simulation.cpp)
target_link_libraries(test_basic_simulation PRIVATE sim)
