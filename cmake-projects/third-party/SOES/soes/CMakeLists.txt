
# NOTE: add headers to make them show up in an IDE
add_library (soes
  esc.c
  esc.h
  esc_coe.c
  esc_coe.h
  esc_foe.c
  esc_foe.h
  esc_eoe.c
  esc_eoe.h
  esc_eep.c
  esc_eep.h
  ecat_slv.c
  ecat_slv.h
  options.h
  ${HAL_SOURCES}
  )

if (CMAKE_C_COMPILER_ID STREQUAL "GNU" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
    target_compile_options(soes PRIVATE -Wno-pedantic -Wno-strict-prototypes -Wno-missing-prototypes -Wno-undef)
endif()

include_directories(${HAL_INCLUDES})

install (TARGETS soes DESTINATION bin)
install (FILES
  esc.h
  esc_coe.h
  esc_foe.h
  esc_eoe.h
  esc_eep.h
  DESTINATION include)
