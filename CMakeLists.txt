cmake_minimum_required(VERSION 3.30)
project(LithoGen LANGUAGES C CXX)

# Refuse to compile on older versions of C++.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)
include(cmake/deps.cmake)

# Automatically recursively gather all source files under src.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${PROJECT_SOURCE_DIR}/src/*)

if (WIN32) # Windows
	# Create the executable, with additional files for Windows.
	add_executable(LithoGen_App ${SOURCE_FILES} ${PROJECT_SOURCE_DIR}/res/resource.rc)

	# Windows specific compiler definitions:
	# WIN32_LEAN_AND_MEAN - Active optional windows api optimisation features.
	# OS_WINDOWS - For the codebase to ask if the program will be compiling to windows.
	# GLFW_EXPOSE_NATIVE_WIN32 - Allows the codebase to access the native windows handle through GLFW.
	target_compile_definitions(LithoGen_App PRIVATE OS_WINDOWS WIN32_LEAN_AND_MEAN)

	# Link the Windows Desktop Window Manager API to interface with the title bar.
	target_link_libraries(LithoGen_App PRIVATE dwmapi)

	# Set the application type to windowed and force the use of the default entry point main().
	#target_link_options(LithoGen_App PRIVATE "/subsystem:windows" PRIVATE "/entry:mainCRTStartup")
elseif (UNIX AND NOT APPLE) # Linux
	# Create the executable.
	add_executable(LithoGen_App ${SOURCE_FILES})
elseif (APPLE) # MacOS
	# Create the executable.
	add_executable(LithoGen_App ${SOURCE_FILES})
endif ()

# GLFW_INCLUDE_NONE - Prevent a compatability issue between Glad and GLFW.
target_compile_definitions(LithoGen_App PRIVATE GLFW_INCLUDE_NONE)

# Embed these files into the executable.
b_embed(LithoGen_App res/shaders/vertex.glsl)
b_embed(LithoGen_App res/shaders/fragment.glsl)

# Manually set the file name of the executable.
set_target_properties(LithoGen_App PROPERTIES OUTPUT_NAME "lithogen")

# Link dependencies to the application.
target_link_libraries(LithoGen_App PRIVATE libglew_static glfw glm imgui microstl nfd stb_image)