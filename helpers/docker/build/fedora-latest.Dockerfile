FROM fedora:latest

RUN dnf install -y \
    gcc-c++ make cmake git pkgconf-pkg-config python3 file \
    autoconf automake libtool dkms \
    alsa-lib-devel libX11-devel libXrandr-devel libXi-devel \
    mesa-libGL-devel mesa-libGLU-devel libXcursor-devel libXinerama-devel \
    gmp-devel openssl-devel libuuid-devel zlib-devel binutils-devel libiberty-devel \
    yaml-cpp-devel tinyxml2-devel ccache gdb \
    doxygen graphviz texlive-latex texlive-collection-fontsrecommended texlive-collection-latexextra \
    clang-tools-extra cppcheck valgrind lcov gcovr tig nodejs npm python3-clang python3-pip \
    && dnf clean all

RUN pip install lizard

WORKDIR /workspace
