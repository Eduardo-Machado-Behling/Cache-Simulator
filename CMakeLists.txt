cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Die if the standard is not available
set(CMAKE_CXX_EXTENSIONS OFF)       # Disable compiler-specific extensions (enforce strictness))

# Project
set(PROJECT_NAME cache_simulator)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin")
set(CMAKE_EXPORT_COMPILE_COMMANDS on)

include_directories("${CMAKE_SOURCE_DIR}/include")
include_directories("${CMAKE_SOURCE_DIR}/vendor/stb")

set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
add_custom_target(copy_assets ALL
				COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSET_DIR}" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/assets"
				COMMENT "Copying assets to output directory"
)
add_compile_options("$<$<CONFIG:Debug>:-D__DEBUG>")


# --- 2. Define Warning Flags Interface ---
add_library(project_warnings INTERFACE)
target_compile_options(project_warnings INTERFACE
				$<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion>
				$<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion>
				$<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

# --- 3. Define "Warnings as Errors" Interface ---
add_library(warnings_as_errors INTERFACE)
target_compile_options(warnings_as_errors INTERFACE
				$<$<CXX_COMPILER_ID:GNU>:-Werror>
				$<$<CXX_COMPILER_ID:Clang>:-Werror>
				$<$<CXX_COMPILER_ID:MSVC>:/WX>
)

# Add the external libraries
set(GLFW_BUILD_WAYLAND OFF)
add_subdirectory(vendor/glfw glfw_build)
add_subdirectory(vendor/glad glad_build)
add_subdirectory(vendor/glm SYSTEM)
add_subdirectory(src)
