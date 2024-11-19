FROM mcr.microsoft.com/devcontainers/base:jammy

RUN apt-get update && apt-get install -y \
    build-essential \
    clang-tidy \
    cmake \
    cppcheck \
    gdb \
    libswt-gtk-4-java \
    mono-complete \
    openjdk-11-jdk \
    python3-pip \
    swig \
    tcl-dev

RUN pip install scons
