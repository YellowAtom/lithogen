
cmake_minimum_required(VERSION 3.30)
project(LithoGen VERSION 1.0.0 LANGUAGES CXX)

# Refuse to compile on older versions of C++.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration for GLFW.
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Configuration for embed.
set(B_PRODUCTION_MODE ON CACHE BOOL "" FORCE)

# Global project precompiler definitions.
add_compile_definitions(GLFW_EXPOSE_NATIVE_WIN32)

# Include project dependencies.
add_subdirectory(deps/glfw-3.4)
add_subdirectory(deps/glad-2.0.8)
add_subdirectory(deps/imgui-1.91.8)
add_subdirectory(deps/glm-1.0.1)
add_subdirectory(deps/embed-1.2.19)

set(SHARED_SOURCE_FILES src/main.cpp
		src/display.cpp
		src/display.h
		src/constants.h
		src/classes/entity.cpp
		src/classes/entity.h
		src/classes/vertex.h
		src/classes/model.h
		src/classes/menu_config.h
		src/classes/display_data.h
		src/classes/display_data.cpp
		src/classes/camera.cpp
		src/classes/camera.h
)

if (WIN32)
	# Create the executable, with additional files for Windows.
	add_executable(App ${SHARED_SOURCE_FILES} res/resource.rc)

	# Windows specific compiler definitions:
	# WIN32_LEAN_AND_MEAN - Active optional windows api optimisation features.
	# OS_WINDOWS - For the codebase to ask if the program will be compiling to windows.
	target_compile_definitions(App PRIVATE OS_WINDOWS WIN32_LEAN_AND_MEAN)

	# Set the application type to windowed and force the use of the default entry point main().
	#target_link_options(App PRIVATE "/subsystem:windows" PRIVATE "/entry:mainCRTStartup")
else()
	# Create the executable.
	add_executable(App ${SHARED_SOURCE_FILES})
endif()

# Embed these files into the executable.
b_embed(App res/shaders/vertex.glsl)
b_embed(App res/shaders/fragment.glsl)

# Manually set the file name of the executable.
set_target_properties(App PROPERTIES OUTPUT_NAME "lithogen")

# Link dependencies to the application.
target_link_libraries(App PRIVATE glfw glad imgui glm)
