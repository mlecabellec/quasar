cmake_minimum_required(VERSION 3.25)
project(resoem_tools LANGUAGES CXX)

# Common include directories
set(TOOLS_INCLUDES
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../third-party/cpp-optparse
    ${CMAKE_CURRENT_SOURCE_DIR}/../../named/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../coretypes/include
)

# ecat-ls
add_executable(ecat-ls src/ecat_ls.cpp ../../third-party/cpp-optparse/OptionParser.cpp)
target_include_directories(ecat-ls PRIVATE ${TOOLS_INCLUDES})
target_link_libraries(ecat-ls PRIVATE resoem quasar_named)

# ecat-diag
add_executable(ecat-diag src/ecat_diag.cpp ../../third-party/cpp-optparse/OptionParser.cpp)
target_include_directories(ecat-diag PRIVATE ${TOOLS_INCLUDES})
target_link_libraries(ecat-diag PRIVATE resoem quasar_named)

# ecat-sdo
add_executable(ecat-sdo src/ecat_sdo.cpp ../../third-party/cpp-optparse/OptionParser.cpp)
target_include_directories(ecat-sdo PRIVATE ${TOOLS_INCLUDES})
target_link_libraries(ecat-sdo PRIVATE resoem quasar_named)

# ecat-top
add_executable(ecat-top src/ecat_top.cpp)
target_include_directories(ecat-top PRIVATE ${TOOLS_INCLUDES})
target_link_libraries(ecat-top PRIVATE resoem quasar_named ftxui::screen ftxui::dom ftxui::component)

# ecat-scan
add_executable(ecat-scan src/ecat_scan.cpp ../../third-party/cpp-optparse/OptionParser.cpp)
target_include_directories(ecat-scan PRIVATE ${TOOLS_INCLUDES})
target_link_libraries(ecat-scan PRIVATE resoem quasar_named tinyxml2)

# ecat-topocheck
add_executable(ecat-topocheck src/ecat_topocheck.cpp ../../third-party/cpp-optparse/OptionParser.cpp)
target_include_directories(ecat-topocheck PRIVATE ${TOOLS_INCLUDES})
target_link_libraries(ecat-topocheck PRIVATE resoem quasar_named tinyxml2)

# ecat-cmp
add_executable(ecat-cmp src/ecat_cmp.cpp ../../third-party/cpp-optparse/OptionParser.cpp)
target_include_directories(ecat-cmp PRIVATE ${TOOLS_INCLUDES})
target_link_libraries(ecat-cmp PRIVATE resoem quasar_named tinyxml2)
