FROM mcr.microsoft.com/devcontainers/base:noble

RUN apt-get update && apt-get install -y \
    build-essential \
    clang-tidy \
    cmake \
    cppcheck \
    gdb \
    libswt-gtk-4-java \
    mono-complete \
    ninja-build \
    openjdk-11-jdk \
    python3-pip \
    swig \
    tcl-dev

RUN pip install --break-system-packages \
    scons \
    conan
