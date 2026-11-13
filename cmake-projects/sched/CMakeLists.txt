add_library(sched
    src/Scheduler.cpp
)

target_include_directories(sched PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(sched PUBLIC ECSS_SMP utils)

add_executable(sched_test test/TestScheduler.cpp)
target_link_libraries(sched_test PRIVATE sched utils sim quasar_test_support GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(sched_test)
