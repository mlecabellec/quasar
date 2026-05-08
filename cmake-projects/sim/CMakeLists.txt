add_library(sim
    src/Simulator.cpp
    src/Resolver.cpp
    src/LinkRegistry.cpp
    src/Model.cpp
    src/LibraryLoader.cpp
    src/TypeRegistry.cpp
    src/Field.cpp
    src/Publication.cpp
    src/PublishOperation.cpp
    src/smp_impl/AccessKind.cpp
    src/smp_impl/AnySimple.cpp
    src/smp_impl/ComponentStateKind.cpp
    src/smp_impl/PrimitiveTypes.cpp
    src/smp_impl/SimulatorStateKind.cpp
    src/smp_impl/Uuid.cpp
    src/smp_impl/Uuids.cpp
    src/smp_impl/ViewKind.cpp
    src/smp_impl/ParameterDirectionKind.cpp
    src/smp_impl/TimeKind.cpp
)

quasar_apply_standards(sim)

target_include_directories(sim PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(sim PUBLIC ECSS_SMP core utils sched ${CMAKE_DL_LIBS})

# Test executable
add_executable(test_basic_simulation test/test_basic_simulation.cpp)
target_link_libraries(test_basic_simulation PRIVATE sim ECSS_SMP)
