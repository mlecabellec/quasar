cmake_minimum_required(VERSION 3.10)
project(ethercat_modules NONE)

include(ExternalProject)

# Preliminary checks for required tools
find_program(AUTORECONF_EXECUTABLE autoreconf)
find_program(MAKE_EXECUTABLE make)
find_program(GCC_EXECUTABLE gcc)

if(NOT AUTORECONF_EXECUTABLE)
    message(FATAL_ERROR "autoreconf not found! generic-ethercat requires autoreconf.")
endif()

if(NOT MAKE_EXECUTABLE)
    message(FATAL_ERROR "make not found! generic-ethercat requires make.")
endif()

if(NOT GCC_EXECUTABLE)
    message(FATAL_ERROR "gcc not found! generic-ethercat requires gcc.")
endif()

ExternalProject_Add(ethercat_external
    URL ${CMAKE_CURRENT_SOURCE_DIR}
    DOWNLOAD_DIR ${CMAKE_CURRENT_BINARY_DIR}/downloads
    SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/src
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/install
    
    # Configure step
    CONFIGURE_COMMAND ./bootstrap
              COMMAND ./configure --prefix=<INSTALL_DIR> --enable-generic=yes --enable-8139too=no
    
    # Build step
    BUILD_COMMAND ${MAKE_EXECUTABLE}
          COMMAND ${MAKE_EXECUTABLE} modules
    
    # Install step (optional, can be empty if handled manually)
    INSTALL_COMMAND ""
    
    BUILD_IN_SOURCE 1
    LOG_CONFIGURE 1
    LOG_BUILD 1
)

add_custom_target(ethercat_modules ALL DEPENDS ethercat_external)
