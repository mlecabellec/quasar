cmake_minimum_required(VERSION 3.10)
project(quasar)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    message(STATUS "Found ccache: ${CCACHE_PROGRAM}")
    set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
else()
    message(STATUS "ccache not found - local build speedups disabled")
endif()

find_program(CLANG_TIDY_EXE NAMES "clang-tidy")

# Function to apply Quasar technical standards to a target
function(quasar_apply_standards target)
    target_compile_features(${target} PUBLIC cxx_std_23)
    target_compile_options(${target} PRIVATE
        -Wall -Wextra -pedantic -Werror
        -Wno-error=shadow
        -Wno-error=unused-parameter
        -Wno-error=conversion
        -Wno-error=sign-conversion
        -Werror=unused-result
        -fstack-usage
    )
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

    if(QUASAR_ENABLE_ASAN)
        target_compile_options(${target} PRIVATE -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(${target} PRIVATE -fsanitize=address)
    endif()

    if(QUASAR_ENABLE_TSAN)
        target_compile_options(${target} PRIVATE -fsanitize=thread)
        target_link_options(${target} PRIVATE -fsanitize=thread)
    endif()

    if(CLANG_TIDY_EXE)
        set_target_properties(${target} PROPERTIES
            CXX_CLANG_TIDY "${CLANG_TIDY_EXE};-checks=modernize-*,cppcoreguidelines-*,bugprone-*,performance-*,readability-magic-numbers;-warnings-as-errors=*;--extra-arg=-Wno-unknown-warning-option;--extra-arg=-std=c++2b"
        )
    endif()
endfunction()

# Options
option(QUASAR_ENABLE_ASAN "Enable AddressSanitizer" OFF)
option(QUASAR_ENABLE_TSAN "Enable ThreadSanitizer" OFF)

enable_testing()

add_subdirectory(cmake-projects)
add_subdirectory(linux-modules)
