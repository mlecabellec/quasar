cmake_minimum_required(VERSION 3.10)
project(quasar_all)

enable_testing()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(third-party)
add_subdirectory(coretypes)
add_subdirectory(named)

# CPack Configuration
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "quasar")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_PACKAGE_CONTACT "Maintainer <maintainer@example.com>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Quasar C++ Libraries")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON) # Attempt to auto-detect dependencies
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Quasar Team")

include(CPack)