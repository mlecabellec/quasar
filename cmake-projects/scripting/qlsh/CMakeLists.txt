project(quasar_qlsh)

# Ensure C++23 is used
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ../../third-party/replxx/include
    ../../third-party/ftxui/include
    ../../third-party/lua
    ../../third-party/sol2/include
    ../../coretypes/include
    ../../named/include
    ../../datalogger/include
    ../include
)

add_executable(qlsh 
    main.cpp
    ShellHighlighter.cpp
    ShellCompleter.cpp
    ShellUtilities.cpp
)

# Link against scripting core, datalogger, and terminal UI libraries
target_link_libraries(qlsh PRIVATE 
    quasar_scripting 
    quasar_datalogger
    replxx 
    screen 
    dom 
    component
)

# Set the output directory to be consistent with sre
set_target_properties(qlsh PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
