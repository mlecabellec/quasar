# QuasarWeb.cmake
# Specialized CMake macros for integrating static web resources and Svelte projects.

include(CMakeParseArguments)

# function quasar_add_static_resources
# @brief Creates a resource library from a list of files using CMRC.
# 
# @param TARGET_NAME The name of the target to create.
# @param NAMESPACE   The C++ namespace for the resources.
# @param FILES       List of files to include.
function(quasar_add_static_resources)
    set(options "")
    set(oneValueArgs TARGET_NAME NAMESPACE)
    set(multiValueArgs FILES)
    cmake_parse_arguments(QSR_RC "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT QSR_RC_TARGET_NAME)
        message(FATAL_ERROR "quasar_add_static_resources: TARGET_NAME is mandatory")
    endif()

    if(NOT QSR_RC_NAMESPACE)
        set(QSR_RC_NAMESPACE "quasar_rc")
    endif()

    # Use CMRC to generate the resource library
    cmrc_add_resource_library(${QSR_RC_TARGET_NAME}
        NAMESPACE ${QSR_RC_NAMESPACE}
        ${QSR_RC_FILES}
    )
    
    # Add an alias for easier usage
    add_library(quasar::${QSR_RC_TARGET_NAME} ALIAS ${QSR_RC_TARGET_NAME})
endfunction()

# function quasar_add_svelte_project
# @brief Builds a Svelte project and integrates its 'dist' output as binary resources.
# 
# @param TARGET_NAME  The name of the resource target to create.
# @param PROJECT_DIR  The directory containing the Svelte project (must have package.json).
# @param NAMESPACE    The C++ namespace for the resources.
function(quasar_add_svelte_project)
    set(options "")
    set(oneValueArgs TARGET_NAME PROJECT_DIR NAMESPACE)
    cmake_parse_arguments(QSR_SV "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT QSR_SV_TARGET_NAME OR NOT QSR_SV_PROJECT_DIR)
        message(FATAL_ERROR "quasar_add_svelte_project: TARGET_NAME and PROJECT_DIR are mandatory")
    endif()

    set(BUILD_DIR "${QSR_SV_PROJECT_DIR}/dist")
    set(STAMP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${QSR_SV_TARGET_NAME}_svelte.stamp")

    # Custom command to build the Svelte project
    add_custom_command(
        OUTPUT "${STAMP_FILE}"
        COMMAND npm install
        COMMAND npm run build
        COMMAND ${CMAKE_COMMAND} -E touch "${STAMP_FILE}"
        WORKING_DIRECTORY "${QSR_SV_PROJECT_DIR}"
        COMMENT "Building Svelte project in ${QSR_SV_PROJECT_DIR}"
        VERBATIM
    )

    # Custom target to trigger the build
    add_custom_target(${QSR_SV_TARGET_NAME}_build ALL
        DEPENDS "${STAMP_FILE}"
    )

    # To handle dynamic files at configure time, we glob the directory if it exists.
    # If it doesn't exist (first build), we provide a fallback (index.html) 
    # and CMRC will be updated on the next build.
    
    set(RES_FILES "")
    if(EXISTS "${BUILD_DIR}")
        file(GLOB_RECURSE FOUND_FILES RELATIVE "${BUILD_DIR}" "${BUILD_DIR}/*")
        foreach(F IN LISTS FOUND_FILES)
            if(NOT IS_DIRECTORY "${BUILD_DIR}/${F}")
                list(APPEND RES_FILES "${BUILD_DIR}/${F}")
            endif()
        endforeach()
    endif()

    if(NOT RES_FILES)
        # Fallback to ensure target creation works
        set(RES_FILES "${BUILD_DIR}/index.html")
    endif()
    
    cmrc_add_resource_library(${QSR_SV_TARGET_NAME}
        NAMESPACE ${QSR_SV_NAMESPACE}
        WHENCE "${BUILD_DIR}"
        ${RES_FILES}
    )

    add_dependencies(${QSR_SV_TARGET_NAME} ${QSR_SV_TARGET_NAME}_build)
    add_library(quasar::${QSR_SV_TARGET_NAME} ALIAS ${QSR_SV_TARGET_NAME})
endfunction()
