cmake_minimum_required(VERSION 3.10)
project(quasar_all)

enable_testing()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(third-party)

# Treat warnings as errors for all Quasar projects (non-third-party)
add_compile_options(-Werror)
add_subdirectory(utils)
add_subdirectory(sched)
# add_subdirectory(smp)
add_subdirectory(sim)
add_subdirectory(coretypes)
add_subdirectory(named)
add_subdirectory(datacodec)
add_subdirectory(scripting)
add_subdirectory(calibration)
add_subdirectory(resoem)
add_subdirectory(logicengine)
add_subdirectory(opcua)
add_subdirectory(sample-models)
add_subdirectory(sim-test)
add_subdirectory(datalogger)
add_subdirectory(manual-tests)

# CPack Configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "quasar")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_CONTACT "Maintainer <maintainer@example.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Quasar C++ Libraries")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # Attempt to auto-detect dependencies
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Quasar Team")

include(CPack)