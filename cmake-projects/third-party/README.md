# Third-Party Libraries

This directory contains the source code for third-party libraries used by the `cmake-projects` build system.

These libraries are included to allow building the project without external dependencies being pre-installed on the system, or to ensure that specific versions of libraries are used.

## Content

-   **googletest** (v1.14.0): Google's C++ testing and mocking framework. Used for unit testing the `coretypes` and `named` projects.
-   **yaml-cpp** (0.8.0): A YAML parser and emitter in C++. Used by the `named` project.
-   **tinyxml2** (10.0.0): A simple, small, efficient, C++ XML parser. Used by the `named` project.

## Licenses

For detailed license information, please refer to the `THIRD-PARTY-LICENSES.md` file in this directory or the individual `LICENSE` files within each library's subdirectory.
