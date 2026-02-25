# The 'utils' library provides core SMP services like EventManager, Logger, and TimeKeeper.
# These services adhere to general requirements of FE-0030, such as thread-safety (FE-0030.8)
# through the use of std::timed_mutex, and exhibit const-correctness in their methods (FE-0030.9).
# However, this library does not implement the specific data types (Number, String, Buffer, BitBuffer)
# or their named variants as described in FE-0030.1-7.
# Furthermore, specific tests and documentation directly linking this module to FE-0030 are not present.

# Contribution to FE-0090 (Datacodec):
# This library provides core SMP services but does not implement the data types (e.g., BitBufferSlice, NamedObject hierarchy)
# or codec functionalities required by FE-0090. The Datacodec library is expected to be located in cmake-projects/datacodec.

add_library(utils
    src/Logger.cpp
    src/TimeKeeper.cpp
    src/EventManager.cpp
)

# This CMakeLists.txt defines the 'utils' library, contributing to the mapping of packages to libraries as per [FE-0080.7.1].

target_include_directories(utils PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(utils PUBLIC ECSS_SMP core)
