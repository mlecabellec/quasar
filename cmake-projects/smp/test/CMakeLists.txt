cmake_minimum_required(VERSION 3.10)

add_executable(StructureTest StructureTest.cpp)
target_link_libraries(StructureTest PRIVATE smp)

add_test(NAME StructureTest COMMAND StructureTest)

add_executable(CoreInterfacesTest CoreInterfacesTest.cpp)
target_link_libraries(CoreInterfacesTest PRIVATE smp)
add_test(NAME CoreInterfacesTest COMMAND CoreInterfacesTest)
add_executable(PublicationTest PublicationTest.cpp)
target_link_libraries(PublicationTest smp)
add_test(NAME PublicationTest COMMAND PublicationTest)

add_executable(AllHeadersTest AllHeadersTest.cpp)
target_link_libraries(AllHeadersTest PRIVATE smp)
add_test(NAME AllHeadersTest COMMAND AllHeadersTest)

add_executable(SimulatorTest SimulatorTest.cpp)
target_link_libraries(SimulatorTest PRIVATE smp)
add_test(NAME SimulatorTest COMMAND SimulatorTest)
