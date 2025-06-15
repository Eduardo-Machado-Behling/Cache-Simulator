# Recursively find all .cpp files in the 'src' directory
file(GLOB_RECURSE SOURCES "*.cpp")

# Add the executable using the list of sources found
message("Sources: ${SOURCES}")
add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

# Link the libraries to your executable
target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm project_warnings )
add_dependencies(${PROJECT_NAME} copy_assets)