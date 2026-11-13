# Machine Resource Constraints & Build Parallelism

Single-threaded build restrictions are **only** applicable on severely resource-limited systems where parallel compilation can exhaust memory or CPU and destabilize the host.

## Rules & Guidelines

1. **Standard Systems (Default)**: Parallel C++ and Java builds (e.g., `cmake --build --parallel`, `make -j$(nproc)`, `ninja`) are permitted.
2. **Limited Systems Only**: When operating on a severely resource-constrained system, cap concurrent threads (`<= 2`) and use single-threaded builds (`-j1`, sequential Maven/Gradle) to prevent resource exhaustion.
