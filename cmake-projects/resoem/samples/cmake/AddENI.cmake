macro(AddENI TARGET ENI_FILE)
    get_filename_component(ENI_NAME ${ENI_FILE} NAME_WE)
    set(ENI_C_FILE "${CMAKE_CURRENT_BINARY_DIR}/${ENI_NAME}.c")

    # In SOEM, eniconv.py prints to stdout. We capture it to a file.
    # The local copy of eniconv.py might behave differently if I copied it from a specific version.
    # Let's verify eniconv.py behavior.
    # But usually it is: python eniconv.py <eni_file> > <c_file>
    
    add_custom_command(
        OUTPUT ${ENI_C_FILE}
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/eniconv.py ${CMAKE_CURRENT_SOURCE_DIR}/${ENI_FILE} > ${ENI_C_FILE}
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${ENI_FILE} ${CMAKE_CURRENT_SOURCE_DIR}/../scripts/eniconv.py
        COMMENT "Generating ${ENI_NAME}.c from ${ENI_FILE}"
    )

    target_sources(${TARGET} PRIVATE ${ENI_C_FILE})
endmacro()
