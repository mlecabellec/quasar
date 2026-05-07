# Quasar Technical Standards Enforcement

if(QUASAR_STANDARDS_INCLUDED)
    return()
endif()
set(QUASAR_STANDARDS_INCLUDED ON)

# Enforce Position Independent Code (PIC) for all targets
# Mandatory when linking shared objects against static libraries on modern toolchains.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

function(quasar_apply_standards target)
    # Treat third-party headers as SYSTEM to suppress their warnings
    get_target_property(target_sources ${target} SOURCES)
    set(is_third_party FALSE)
    foreach(source ${target_sources})
        if(source MATCHES "third-party")
            set(is_third_party TRUE)
            break()
        endif()
    endforeach()

    # Enforce C++23
    target_compile_features(${target} PUBLIC cxx_std_23)
    
    # Strict warnings and security flags only for non-third-party code
    if(NOT is_third_party)
        target_compile_options(${target} PRIVATE
            -Wall -Wextra -pedantic -Werror
            -Wno-error=shadow
            -Wno-error=unused-parameter
            -Wno-error=conversion
            -Wno-error=sign-conversion
            -Werror=unused-result
            -fstack-usage
        )
    else()
        # For third-party, we want to BE QUIET
        target_compile_options(${target} PRIVATE -w)
    endif()
    
    # Enable assertions in standard library
    target_compile_definitions(${target} PRIVATE _GLIBCXX_ASSERTIONS)

    # Treat third-party headers as SYSTEM to suppress their warnings
    get_target_property(include_dirs ${target} INCLUDE_DIRECTORIES)
    if(include_dirs)
        foreach(dir ${include_dirs})
            if(dir MATCHES "third-party")
                set_property(TARGET ${target} APPEND PROPERTY INTERFACE_SYSTEM_INCLUDE_DIRECTORIES ${dir})
            endif()
        endforeach()
    endif()

    # AddressSanitizer support
    if(QUASAR_ENABLE_ASAN)
        target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endif()

    # ThreadSanitizer support
    if(QUASAR_ENABLE_TSAN)
        target_compile_options(${target} PRIVATE -fsanitize=thread)
        target_link_options(${target} PRIVATE -fsanitize=thread)
    endif()

    # Clang-Tidy integration
    if(CLANG_TIDY_EXE)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=modernize-*,cppcoreguidelines-*,bugprone-*,performance-*,readability-magic-numbers,-modernize-use-auto,-modernize-use-trailing-return-type;--extra-arg=-Wno-unknown-warning-option;--extra-arg=-std=c++2b"
        )
    endif()
endfunction()
