FROM debian:bookworm

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
    curl ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Install Gitea act_runner
RUN curl -L https://gitea.com/gitea/act_runner/releases/download/v0.2.11/act_runner-0.2.11-linux-amd64 -o /usr/local/bin/act_runner \
    && chmod +x /usr/local/bin/act_runner

WORKDIR /runner
ENTRYPOINT ["/usr/local/bin/act_runner", "daemon"]
