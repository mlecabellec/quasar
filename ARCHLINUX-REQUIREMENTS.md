# Arch Linux Requirements for Quasar

This document lists the Arch Linux packages required to build, test, and document the Quasar project. These requirements are equivalents to the Debian packages listed in `DEBIAN-REQUIREMENTS.md`.

## 1. Essential Build Tools

In Arch Linux, essential build tools are bundled in the `base-devel` group.

*   `base-devel`: Group containing `gcc`, `make`, `pkgconf`, `binutils`, etc.
*   `cmake`: The primary build system generator.
*   `git`: For version control and fetching submodules/content.
*   `python`: Required for various scripts and build helpers.
*   `file`: Utility to determine file types, used in CI.

## 2. Kernel Module Development

Required for building the `linux-modules` (EtherCAT, SOEM, SOES).

*   `linux-headers`: Header files for the Linux kernel.
*   `autoconf`: Configuration script builder.
*   `automake`: Makefile generator.
*   `libtool`: Generic library support script.
*   `dkms`: Dynamic Kernel Module Support.

## 3. Third-Party Library Dependencies

On Arch Linux, headers are usually included in the main package.

### Raylib (Multimedia & GUI)
*   `alsa-lib`: ALSA sound library.
*   `libx11`: X11 client-side library.
*   `libxrandr`: X11 Resize and Rotate extension.
*   `libxi`: X11 Input extension.
*   `mesa`: OpenGL support.
*   `glu`: OpenGL Utility Library.
*   `libxcursor`: X11 Cursor management.
*   `libxinerama`: X11 Xinerama extension.

### SymEngine (Symbolic Math)
*   `gmp`: GNU Multiple Precision Arithmetic Library.

### Utilities & Security
*   `openssl`: OpenSSL libraries and development files.
*   `util-linux-libs`: Universally Unique ID library (provides libuuid).
*   `zlib`: Compression library.
*   `binutils`: Binary utilities (provides libiberty).

### Optional / Optimization
*   `yaml-cpp`: YAML parser and emitter.
*   `tinyxml2`: XML parser.

## 4. Documentation Tools

*   `doxygen`: Documentation generator.
*   `graphviz`: Graph visualization software.
*   `texlive-latex`: Basic LaTeX distribution.
*   `texlive-fontsrecommended`: Recommended fonts for LaTeX.
*   `texlive-latexextra`: Additional LaTeX packages.

## 5. Verification & Quality Tools

*   `clang`: Includes `clang-tidy` and other LLVM tools.
*   `cppcheck`: Static analysis for C++.
*   `valgrind`: Dynamic memory and concurrency analysis.
*   `lcov` / `gcovr`: Code coverage analysis.
*   `tig`: Terminal-based repository browser.
*   `nodejs` / `npm`: For `commitlint`.
*   `python-clang`: Python bindings for libclang.
*   `python-lizard`: Code complexity analyzer (available in AUR).

## 6. Quick Install Command

You can install the official repository requirements using the following command:

```bash
sudo pacman -S --needed \
    base-devel cmake git python file \
    linux-headers autoconf automake libtool dkms \
    alsa-lib libx11 libxrandr libxi mesa glu libxcursor libxinerama \
    gmp openssl util-linux-libs zlib binutils yaml-cpp tinyxml2 \
    doxygen graphviz texlive-latex texlive-fontsrecommended texlive-latexextra \
    clang cppcheck valgrind lcov gcovr tig nodejs npm python-clang
```

### AUR Packages
The following tool may need to be installed from the AUR (e.g., using `yay`):
```bash
yay -S python-lizard
```
Alternatively, install it via pip: `pip install lizard`.
