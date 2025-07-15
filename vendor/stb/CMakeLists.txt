cmake_minimum_required(VERSION 3.10)

project(MyImageLoader CXX)

# Add the directory containing stb_image.h to the include paths
# This makes it so you can use #include "stb_image.h" in your source files
# without needing to specify the full relative path.
# PRIVATE means this include directory is only for this target.
target_include_directories(MyImageLoader PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs/stb)

# Add your executable
# This will compile main.cpp into an executable named MyImageLoader
add_executable(MyImageLoader main.cpp)

# (Optional but good practice for larger projects)
# If you were using other libraries (like OpenGL, GLFW, etc.), you would link them here:
# find_package(OpenGL REQUIRED)
# find_package(GLFW REQUIRED)
# target_link_libraries(MyImageLoader PRIVATE OpenGL::GL GLFW::GLFW)
