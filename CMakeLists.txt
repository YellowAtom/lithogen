cmake_minimum_required(VERSION 3.30)
project(LithoGen LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)
include(cmake/deps.cmake) # Manages project dependencies.

# Automatically recursively gather all source files under src.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*)

add_executable(LithoGen_App ${SOURCE_FILES})

# GLFW_INCLUDE_NONE - Prevent a compatability issue between Glad and GLFW.
target_compile_definitions(LithoGen_App PRIVATE "GLFW_INCLUDE_NONE")

# Embed these files into the executable.
b_embed(LithoGen_App "res/shaders/vertex.glsl")
b_embed(LithoGen_App "res/shaders/fragment.glsl")

# Manually set the file name of the executable.
set_target_properties(LithoGen_App PROPERTIES OUTPUT_NAME "lithogen")

# Link dependencies to the application.
target_link_libraries(LithoGen_App PRIVATE glad glfw glm imgui microstl nfd stb_image)

if (WIN32) # Windows
	target_sources(LithoGen_App PRIVATE "res/resource.rc")

	# WIN32_LEAN_AND_MEAN - Active optional windows api optimisation features.
	target_compile_definitions(LithoGen_App PRIVATE "WIN32_LEAN_AND_MEAN")

	# Set the application type to windowed and force the use of the default entry point main().
	#target_link_options(LithoGen_App PRIVATE "/subsystem:windows" PRIVATE "/entry:mainCRTStartup")
endif ()