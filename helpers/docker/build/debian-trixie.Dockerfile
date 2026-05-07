FROM debian:trixie

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake git pkg-config python3 file \
    autoconf automake libtool dkms \
    libasound2-dev libx11-dev libxrandr-dev libxi-dev \
    libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev \
    libgmp-dev libssl-dev libsasl2-dev libzstd-dev uuid-dev binutils-dev zlib1g-dev libiberty-dev \
    libcurl4-openssl-dev gdb ccache \
    libyaml-cpp-dev libtinyxml2-dev libjsoncons-dev \
    doxygen graphviz texlive-latex-base texlive-fonts-recommended texlive-latex-extra \
    clang-tidy cppcheck valgrind lcov gcovr tig nodejs npm python3-clang \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /workspace
