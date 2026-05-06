FROM archlinux:latest

RUN pacman -Sy --needed --noconfirm \
    base-devel cmake git python file \
    autoconf automake libtool dkms \
    alsa-lib libx11 libxrandr libxi mesa glu libxcursor libxinerama \
    gmp openssl util-linux-libs zlib binutils yaml-cpp tinyxml2 ccache gdb \
    doxygen graphviz texlive-latex texlive-fontsrecommended texlive-latexextra \
    clang cppcheck valgrind lcov gcovr tig nodejs npm python-clang python-pip \
    curl ca-certificates \
    && pacman -Scc --noconfirm

RUN pip install --break-system-packages lizard

# Install Gitea act_runner
RUN curl -L https://gitea.com/gitea/act_runner/releases/download/v0.2.11/act_runner-0.2.11-linux-amd64 -o /usr/local/bin/act_runner \
    && chmod +x /usr/local/bin/act_runner

WORKDIR /runner
ENTRYPOINT ["/usr/local/bin/act_runner", "daemon"]
