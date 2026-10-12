# Linux build/test/diff image: ubuntu 26.04 + clang (libFuzzer) + Fcitx5 dev + go/python3 for tools/diff.
# docker build [--platform linux/amd64] -f tools/docker/linux-dev.Dockerfile -t sentype-dev tools/docker
FROM ubuntu:26.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential clang libclang-rt-dev cmake ninja-build git file curl ca-certificates perl \
    libfcitx5core-dev libfcitx5utils-dev libfcitx5config-dev fcitx5-modules-dev fcitx5 \
    dpkg-dev xvfb golang-go python3 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
