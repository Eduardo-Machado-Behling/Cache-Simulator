# Use Ubuntu as the base image
FROM ubuntu:22.04

# Avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary dependencies for OpenGL, GLFW, GLM, and X11
RUN apt-get update && apt-get install -y \
    software-properties-common # For add-apt-repository

# Add a PPA for newer GCC versions
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    xorg-dev \
    libglfw3-dev \
    libglm-dev \
    libx11-dev \
    libxrandr-dev \
    libxinerama-dev \
    libxcursor-dev \
    libxi-dev \
    libxkbcommon-dev \
    g++-13 # Install GCC/G++ 13

# Set up a working directory
WORKDIR /app

# Copy the application source code into the container
COPY . /app

# Create a build directory and compile the application
RUN rm -rf build && \
    # Explicitly tell CMake to use g++-13
    cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-13 && \
    cmake --build build

# Command to run the application
CMD ["./bin/cache_simulator"]
