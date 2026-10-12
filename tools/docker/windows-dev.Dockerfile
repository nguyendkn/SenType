# Windows cross-build + NSIS installer + wine regsvr32/installer check (x86-64 image; on Apple Silicon use --platform linux/amd64).
# docker build --platform linux/amd64 -f tools/docker/windows-dev.Dockerfile -t sentype-win tools/docker
FROM ubuntu:26.04
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && apt-get install -y --no-install-recommends \
    cmake ninja-build g++-mingw-w64-x86-64-posix g++-mingw-w64-i686-posix wine64 xvfb xauth \
    ca-certificates nsis && rm -rf /var/lib/apt/lists/*
WORKDIR /src
