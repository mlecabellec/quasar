# Debian Requirements for Quasar

This document lists the Debian packages required to build, test, and document the Quasar project. These requirements are based on the analysis of `CMakeLists.txt`, CI configurations (`.gitlab-ci.yml`, `.github/workflows/ci.yml`), and documentation settings.

## 1. Essential Build Tools

These tools are required for the basic compilation and configuration of the project.

*   `build-essential`: Includes `make`, `g++`, and other standard build tools.
*   `cmake`: The primary build system generator.
*   `git`: For version control and fetching submodules/content.
*   `pkg-config`: Helper tool used when compiling applications and libraries.
*   `python3`: Required for various scripts and build helpers.
*   `file`: Utility to determine file types, used in CI.

## 2. Kernel Module Development

Required for building the `linux-modules` (EtherCAT, SOEM, SOES).

*   `linux-headers-$(uname -r)`: Header files for the running Linux kernel (or `linux-headers-amd64` / `linux-headers-generic` for CI environments).
*   `autoconf`: Configuration script builder (required for EtherCAT bootstrap).
*   `automake`: Makefile generator (required for EtherCAT bootstrap).
*   `libtool`: Generic library support script (required for EtherCAT bootstrap).
*   `dkms`: (Optional but recommended) Dynamic Kernel Module Support.

## 3. Third-Party Library Dependencies

These packages provide the necessary system libraries for the third-party components used in Quasar.

### Raylib (Multimedia & GUI)
Raylib is vendored but links against system libraries on Linux.
*   `libasound2-dev`: ALSA sound library.
*   `libx11-dev`: X11 client-side library.
*   `libxrandr-dev`: X11 Resize and Rotate extension.
*   `libxi-dev`: X11 Input extension.
*   `libgl1-mesa-dev`: OpenGL support.
*   `libglu1-mesa-dev`: OpenGL Utility Library.
*   `libxcursor-dev`: X11 Cursor management.
*   `libxinerama-dev`: X11 Xinerama extension.

### SymEngine (Symbolic Math)
*   `libgmp-dev`: GNU Multiple Precision Arithmetic Library (required by SymEngine configuration).

### Utilities & Security
*   `libssl-dev`: OpenSSL development files (required for secure communication).
*   `uuid-dev`: Universally Unique ID library.
*   `zlib1g-dev`: Compression library.
*   `binutils-dev`: Binary utilities development files (often for symbol resolution and debugging).
*   `libiberty-dev`: Library of GNU utility functions.

### Optional / Optimization
The build system can use system versions of these libraries if found, speeding up the build and avoiding network fetches.
*   `libyaml-cpp-dev`: YAML parser and emitter.
*   `libtinyxml2-dev`: XML parser.
*   `libjsoncons-dev`: JSON construction library.

## 4. Documentation Tools

Required if you intend to generate the project documentation (HTML or PDF).

*   `doxygen`: Documentation generator.
*   `graphviz`: Graph visualization software (used by Doxygen for diagrams).
*   `texlive-latex-base`: Basic LaTeX distribution for PDF generation.
*   `texlive-fonts-recommended`: Recommended fonts for LaTeX.
*   `texlive-latex-extra`: Additional LaTeX packages often used by Doxygen.

## 5. Verification & Quality Tools
Mandatory for compliance with Quasar Technical Standards (CS-0010 to CS-0060).

*   `clang-tidy`: Linter and static analyzer.
*   `cppcheck`: Static analysis for C++.
*   `valgrind`: Dynamic memory and concurrency analysis.
*   `lcov` / `gcovr`: Code coverage analysis.
*   `lizard`: Code complexity analysis (Python: `pip install lizard`).
*   `tig`: Terminal-based repository browser / visualization.
*   `nodejs` / `npm`: For `commitlint` (Conventional Commits).

## 6. Quick Install Command

You can install all the requirements using the following command:

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential cmake git pkg-config python3 file \
    linux-headers-$(uname -r) autoconf automake libtool dkms \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
    libgmp-dev libssl-dev uuid-dev zlib1g-dev binutils-dev libiberty-dev \
    libyaml-cpp-dev libtinyxml2-dev libjsoncons-dev \
    doxygen graphviz texlive-latex-base texlive-fonts-recommended texlive-latex-extra \
    clang-tidy cppcheck valgrind lcov gcovr tig nodejs npm python3-clang
```

*Note: If you are setting up a CI environment (e.g., Docker), replace `linux-headers-$(uname -r)` with the specific header package for your target kernel (e.g., `linux-headers-amd64` on Debian).*
