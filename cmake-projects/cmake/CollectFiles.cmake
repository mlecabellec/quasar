# CollectFiles.cmake
# Helper script to list all files in a directory for CMRC integration.

file(GLOB_RECURSE FOUND_FILES RELATIVE "${BASE_DIR}" "${BASE_DIR}/*")

set(FILES_LIST "")
foreach(FILE IN LISTS FOUND_FILES)
    # Filter out directories
    if(NOT IS_DIRECTORY "${BASE_DIR}/${FILE}")
        list(APPEND FILES_LIST "${FILE}")
    endif()
endforeach()

# Write to a temporary file that can be included by CMake
file(WRITE "${OUTPUT_FILE}" "set(${VAR_NAME} ${FILES_LIST})\n")
