cmake_minimum_required(VERSION 3.10)
project(lua C)

# Source files extracted from the makefile
set(CORE_SRC
    lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c llex.c
    lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c ltable.c
    ltm.c lundump.c lvm.c lzio.c ltests.c
)

set(AUX_SRC
    lauxlib.c
)

set(LIB_SRC
    lbaselib.c ldblib.c liolib.c lmathlib.c loslib.c ltablib.c lstrlib.c
    lutf8lib.c loadlib.c lcorolib.c linit.c
)

set(LUA_SRC
    lua.c
)

# Define the library
add_library(liblua STATIC ${CORE_SRC} ${AUX_SRC} ${LIB_SRC})
target_include_directories(liblua PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Compiler options from makefile
if(UNIX)
    target_compile_definitions(liblua PRIVATE LUA_USE_LINUX)
    target_link_libraries(liblua PRIVATE m dl)
endif()

# Define the executable
add_executable(lua ${LUA_SRC})
target_link_libraries(lua PRIVATE liblua)
if(UNIX)
    target_link_libraries(lua PRIVATE m dl)
endif()
