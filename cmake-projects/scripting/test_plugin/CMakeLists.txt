project(quasar_test_plugin)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Create the shared library plugin
add_library(test_plugin SHARED TestPlugin.cpp)

# Link dependencies
target_link_libraries(test_plugin PUBLIC 
    quasar_scripting
    sol2
)
