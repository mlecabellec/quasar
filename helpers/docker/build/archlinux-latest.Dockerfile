FROM archlinux:latest

RUN pacman -Sy --needed --noconfirm \
    base-devel cmake git python file \
    autoconf automake libtool dkms \
    alsa-lib libx11 libxrandr libxi mesa glu libxcursor libxinerama \
    gmp openssl util-linux-libs zlib binutils yaml-cpp tinyxml2 ccache gdb \
    doxygen graphviz texlive-latex texlive-fontsrecommended texlive-latexextra \
    clang cppcheck valgrind lcov gcovr tig nodejs npm python-clang python-pip \
    && pacman -Scc --noconfirm

RUN pip install --break-system-packages lizard

WORKDIR /workspace
