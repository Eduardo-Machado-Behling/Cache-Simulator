cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the external libraries
add_subdirectory(vendor/glfw)
add_subdirectory(vendor/glad)

# Add your executable
add_executable(YourApp src/main.cpp)

# Link the libraries to your executable
target_link_libraries(YourApp PRIVATE glfw glad)