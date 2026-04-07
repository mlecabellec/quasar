add_library(quasar_datalogger_plugin SHARED src/DataLoggerPlugin.cpp)

target_include_directories(quasar_datalogger_plugin PRIVATE
    ../../include
    ../../../coretypes/include
    ../../../named/include
    ../../../datalogger/include
    ../../../third-party/lua
    ../../../third-party/sol2/include
)

target_link_libraries(quasar_datalogger_plugin PRIVATE
    quasar_scripting
    quasar_datalogger
)

target_compile_definitions(quasar_datalogger_plugin PRIVATE QUASAR_PLUGIN_EXPORTS)
