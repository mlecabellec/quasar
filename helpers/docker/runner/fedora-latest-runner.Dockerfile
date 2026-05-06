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
    curl ca-certificates \
    && dnf clean all

RUN pip install lizard

# Install Gitea act_runner
RUN curl -L https://gitea.com/gitea/act_runner/releases/download/v0.2.11/act_runner-0.2.11-linux-amd64 -o /usr/local/bin/act_runner \
    && chmod +x /usr/local/bin/act_runner

WORKDIR /runner
ENTRYPOINT ["/usr/local/bin/act_runner", "daemon"]
