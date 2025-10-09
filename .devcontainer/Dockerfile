FROM ghcr.io/breadfish64/clang-p2996:latest

LABEL org.opencontainers.image.title="AdventOfCode"
LABEL org.opencontainers.image.vendor="BreadFish64"

SHELL ["/bin/bash", "-c"]

RUN apt-get update && apt-get install -y locales 7zip zstd curl git python3 python-is-python3 cmake ninja-build z3 gdb
