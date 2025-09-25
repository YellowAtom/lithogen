cmake_minimum_required(VERSION 3.30)
project(LithoGen LANGUAGES C CXX)

# Refuse to compile on older versions of C++.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(cmake/CPM.cmake)

# Configuration for GLFW.
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Configuration for battery embed.
set(B_PRODUCTION_MODE ON CACHE BOOL "" FORCE)

# Dependencies
CPMAddPackage("gh:batterycenter/embed#v1.2.19")
CPMAddPackage("gh:glfw/glfw#3.4")
CPMAddPackage("gh:g-truc/glm#1.0.1")
CPMAddPackage("gh:btzy/nativefiledialog-extended#v1.2.1")

CPMAddPackage(
		NAME microstl
		GIT_REPOSITORY "https://github.com/cry-inc/microstl.git"
		GIT_TAG "ec3868a14d8eff40f7945b39758edf623f609b6f"
		DOWNLOAD_ONLY YES
)

if (microstl_ADDED)
	add_library(microstl INTERFACE)
	target_include_directories(microstl INTERFACE ${microstl_SOURCE_DIR}/include)
endif ()

CPMAddPackage(
		NAME imgui
		GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
		GIT_TAG "v1.92.3"
		DOWNLOAD_ONLY YES
)

if (imgui_ADDED)
	file(GLOB imgui_ROOT_FILES ${imgui_SOURCE_DIR}/*.cpp ${imgui_SOURCE_DIR}/*.h)
	set(imgui_BACKEND_FILES
			${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
			${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.h
			${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
			${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.h
			${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3_loader.h
	)

	# Create alternate directory to stored desired files to avoid polluting global project includes.
	file(MAKE_DIRECTORY ${imgui_SOURCE_DIR}/include/backends)

	file(COPY ${imgui_ROOT_FILES} DESTINATION ${imgui_SOURCE_DIR}/include)

	if (EXISTS ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
		file(COPY ${imgui_BACKEND_FILES} DESTINATION ${imgui_SOURCE_DIR}/include/backends)
	endif ()

	file(REMOVE ${imgui_ROOT_FILES})
	file(REMOVE ${imgui_BACKEND_FILES})

	file(GLOB_RECURSE imgui_FINAL_SOURCE_FILES ${imgui_SOURCE_DIR}/include/*.cpp)

	add_library(imgui ${imgui_FINAL_SOURCE_FILES})
	target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/include)
	target_link_libraries(imgui PRIVATE glfw)
endif ()

CPMAddPackage(
		NAME stb
		GIT_REPOSITORY "https://github.com/nothings/stb.git"
		GIT_TAG "fede005abaf93d9d7f3a679d1999b2db341b360f"
		DOWNLOAD_ONLY YES
)

if (stb_ADDED)
	# Move implementation to static binary.
	file(WRITE ${stb_SOURCE_DIR}/stb_image.c "#define STB_IMAGE_IMPLEMENTATION\n#include \"stb_image.h\"")

	# Ensure source files don't pollute project includes.
	file(MAKE_DIRECTORY ${stb_SOURCE_DIR}/includes)

	if (EXISTS ${stb_SOURCE_DIR}/stb_image.h)
		file(COPY ${stb_SOURCE_DIR}/stb_image.h DESTINATION ${stb_SOURCE_DIR}/includes)
		file(REMOVE ${stb_SOURCE_DIR}/stb_image.h)
	endif ()

	add_library(stb_image ${stb_SOURCE_DIR}/stb_image.c)
	target_include_directories(stb_image PUBLIC ${stb_SOURCE_DIR}/includes)
endif ()

# Bundled Dependencies
add_subdirectory(thirdparty/glad-2.0.8)

# Automatically recursively gather all source files under src.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*)

if (WIN32) # Windows
	# Create the executable, with additional files for Windows.
	add_executable(LithoGen_App ${SOURCE_FILES} res/resource.rc)

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
target_link_libraries(LithoGen_App PRIVATE glad glfw glm imgui microstl nfd stb_image)