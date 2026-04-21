project(quasar_scripting)

# Ensure C++23 is used
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(
    include
    ../third-party/lua
    ../third-party/sol2/include
    ../third-party/asio/asio/include
    ../coretypes/include
    ../named/include
)

# Source files
set(SOURCES
    src/LuaEngine.cpp
    src/Environment.cpp
    src/TypeBindings.cpp
    src/RegistryBindings.cpp
    src/ScriptExecutor.cpp
    src/ScriptableNamedObject.cpp
    src/LuaService.cpp
    src/ObjectTracker.cpp
    src/ScriptManager.cpp
    src/ScriptConsole.cpp
    src/PluginLoader.cpp
    src/NamedLuaMethod.cpp
    src/LuaObserver.cpp
)

# Header files (for IDEs)
set(HEADERS
    include/quasar/scripting/LuaEngine.hpp
    include/quasar/scripting/Environment.hpp
    include/quasar/scripting/TypeBindings.hpp
    include/quasar/scripting/RegistryBindings.hpp
    include/quasar/scripting/LuaFuture.hpp
    include/quasar/scripting/ScriptExecutor.hpp
    include/quasar/scripting/ScriptableNamedObject.hpp
    include/quasar/scripting/LuaService.hpp
    include/quasar/scripting/ObjectTracker.hpp
    include/quasar/scripting/ScriptManager.hpp
    include/quasar/scripting/ScriptConsole.hpp
    include/quasar/scripting/PluginLoader.hpp
    include/quasar/scripting/PluginContract.hpp
    include/quasar/scripting/NamedLuaMethod.hpp
    include/quasar/scripting/LuaObserver.hpp
)

# Create the library
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})

# Link dependencies
target_link_libraries(${PROJECT_NAME} PUBLIC 
    liblua 
    sol2 
    quasar_coretypes 
    quasar_named
)

# Export include directories for targets that link against this
target_include_directories(${PROJECT_NAME} PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_subdirectory(test)
add_subdirectory(runner)
add_subdirectory(qlsh)
add_subdirectory(test_plugin)
add_subdirectory(plugins/net)
add_subdirectory(plugins/zmq)
add_subdirectory(plugins/opcua)
add_subdirectory(plugins/datalogger)
add_subdirectory(plugins/resoem)
