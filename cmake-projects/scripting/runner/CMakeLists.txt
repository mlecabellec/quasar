add_executable(quasar-script-runner main.cpp)
target_link_libraries(quasar-script-runner PUBLIC quasar_scripting)
set_target_properties(quasar-script-runner PROPERTIES OUTPUT_NAME "sre")
