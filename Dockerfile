# Use Ubuntu as the base image
FROM ubuntu:22.04

# Avoid interactive prompts during installation
ENV DEBIAN_FRONTEND=noninteractive

# Install necessary dependencies for OpenGL, GLFW, GLM, and X11
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
    libxkbcommon-dev 

# Set up a working directory
WORKDIR /app

# Copy the application source code into the container
COPY . /app

# Create a build directory and compile the application
RUN rm -rf build && \
    cmake -S . -B build && \
    cmake --build build 

# Command to run the application
CMD ["./bin/cache-simulator"]

