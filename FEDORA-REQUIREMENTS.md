# Fedora Requirements for Quasar

This document lists the Fedora packages required to build, test, and document the Quasar project. These requirements are equivalents to the Debian packages listed in `DEBIAN-REQUIREMENTS.md`.

## 1. Essential Build Tools

These tools are required for the basic compilation and configuration of the project.

*   `gcc-c++`: The GNU C++ compiler.
*   `make`: The GNU build automation tool.
*   `cmake`: The primary build system generator.
*   `git`: For version control and fetching submodules/content.
*   `pkgconf-pkg-config`: Helper tool used when compiling applications and libraries.
*   `python3`: Required for various scripts and build helpers.
*   `file`: Utility to determine file types, used in CI.

## 2. Kernel Module Development

Required for building the `linux-modules` (EtherCAT, SOEM, SOES).

*   `kernel-devel-$(uname -r)`: Header files for the running Linux kernel.
*   `autoconf`: Configuration script builder.
*   `automake`: Makefile generator.
*   `libtool`: Generic library support script.
*   `dkms`: Dynamic Kernel Module Support.

## 3. Third-Party Library Dependencies

### Raylib (Multimedia & GUI)
*   `alsa-lib-devel`: ALSA sound library development files.
*   `libX11-devel`: X11 client-side library development files.
*   `libXrandr-devel`: X11 Resize and Rotate extension development files.
*   `libXi-devel`: X11 Input extension development files.
*   `mesa-libGL-devel`: OpenGL support development files.
*   `mesa-libGLU-devel`: OpenGL Utility Library development files.
*   `libXcursor-devel`: X11 Cursor management development files.
*   `libXinerama-devel`: X11 Xinerama extension development files.

### SymEngine (Symbolic Math)
*   `gmp-devel`: GNU Multiple Precision Arithmetic Library development files.

### Utilities & Security
*   `openssl-devel`: OpenSSL development files.
*   `libuuid-devel`: Universally Unique ID library development files.
*   `zlib-devel`: Compression library development files.
*   `binutils-devel`: Binary utilities development files.

### Optional / Optimization
*   `yaml-cpp-devel`: YAML parser and emitter development files.
*   `tinyxml2-devel`: XML parser development files.

## 4. Documentation Tools

*   `doxygen`: Documentation generator.
*   `graphviz`: Graph visualization software.
*   `texlive-latex`: Basic LaTeX distribution.
*   `texlive-collection-fontsrecommended`: Recommended fonts for LaTeX.
*   `texlive-collection-latexextra`: Additional LaTeX packages.

## 5. Verification & Quality Tools

*   `clang-tools-extra`: Includes `clang-tidy`.
*   `cppcheck`: Static analysis for C++.
*   `valgrind`: Dynamic memory and concurrency analysis.
*   `lcov` / `gcovr`: Code coverage analysis.
*   `tig`: Terminal-based repository browser.
*   `nodejs` / `npm`: For `commitlint`.
*   `python3-clang`: Python bindings for libclang.
*   `lizard`: Python code complexity analyzer (`pip install lizard`).

## 6. Quick Install Command

You can install all the requirements using the following command:

```bash
sudo dnf install -y \
    gcc-c++ make cmake git pkgconf-pkg-config python3 file \
    kernel-devel-$(uname -r) autoconf automake libtool dkms \
    alsa-lib-devel libX11-devel libXrandr-devel libXi-devel \
    mesa-libGL-devel mesa-libGLU-devel libXcursor-devel libXinerama-devel \
    gmp-devel openssl-devel libuuid-devel zlib-devel binutils-devel \
    yaml-cpp-devel tinyxml2-devel \
    doxygen graphviz texlive-latex texlive-collection-fontsrecommended texlive-collection-latexextra \
    clang-tools-extra cppcheck valgrind lcov gcovr tig nodejs npm python3-clang
pip install lizard
```
