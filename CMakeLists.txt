
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

# Global project precompiler definitions.
add_compile_definitions(GLFW_EXPOSE_NATIVE_WIN32)

# Include project dependencies.
add_subdirectory(deps/glfw-3.4)
add_subdirectory(deps/glad-2.0.8)
add_subdirectory(deps/imgui-1.91.7)
add_subdirectory(deps/glm-1.0.1)

set(SHARED_SOURCE_FILES src/main.cpp)

if (WIN32)
	# Create the executable, with additional files for Windows.
	add_executable(App ${SHARED_SOURCE_FILES} res/resource.rc)

	# Windows specific compiler definitions:
	# WIN32_LEAN_AND_MEAN - Active optional windows api optimisation features.
	# OS_WINDOWS - For the codebase to ask if the program will be compiling to windows.
	target_compile_definitions(App PRIVATE OS_WINDOWS WIN32_LEAN_AND_MEAN)

	# Link the Windows Desktop Window Manager API to interface with the title bar.
	target_link_libraries(App PRIVATE dwmapi)

	# Set the application type to windowed and force the use of the default entry point main().
	#target_link_options(App PRIVATE "/subsystem:windows" PRIVATE "/entry:mainCRTStartup")
else()
	# Create the executable.
	add_executable(App ${SHARED_SOURCE_FILES})
endif()

# Manually set the file name of the executable.
set_target_properties(App PROPERTIES OUTPUT_NAME "lithogen")

# Link dependencies to the application.
target_link_libraries(App PRIVATE glfw glad imgui glm)
