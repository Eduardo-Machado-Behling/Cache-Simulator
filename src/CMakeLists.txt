# 1. Gather all .cpp files
file(GLOB_RECURSE SOURCES
    CONFIGURE_DEPENDS
    "*.cpp"
)

# --- DEBUG: Print all found files BEFORE filtering ---
message(STATUS "BEFORE filter: ${SOURCES}")

if(NOT BUILD_GUI)
	list(FILTER SOURCES EXCLUDE REGEX ".*/Frontend/Simulator/.*")
endif(NOT BUILD_GUI)

# --- DEBUG: Print the list AFTER filtering ---
message(STATUS "AFTER filter: ${SOURCES}")

add_executable(${PROJECT_NAME} main.cpp ${SOURCES})

# Link the libraries to your executable
if(BUILD_GUI)
	target_compile_definitions(${PROJECT_NAME} PRIVATE "BUILD_GUI")
	target_link_libraries(${PROJECT_NAME} PRIVATE glfw glad glm project_warnings )
endif(BUILD_GUI)
add_dependencies(${PROJECT_NAME} copy_assets)
