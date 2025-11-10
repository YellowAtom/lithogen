cmake_minimum_required(VERSION 3.30)
project(LithoGen VERSION 1.0.1 LANGUAGES CXX)

# Refuse to compile on older versions of C++.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration for GLFW.
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Configuration for battery embed.
set(B_PRODUCTION_MODE ON CACHE BOOL "" FORCE)

# Include project dependencies.
add_subdirectory(deps/battery-embed-1.2.19 EXCLUDE_FROM_ALL)
add_subdirectory(deps/glad-2.0.8 EXCLUDE_FROM_ALL)
add_subdirectory(deps/glfw-3.4 EXCLUDE_FROM_ALL)
add_subdirectory(deps/glm-1.0.1 EXCLUDE_FROM_ALL)
add_subdirectory(deps/imgui-1.91.8 EXCLUDE_FROM_ALL)
add_subdirectory(deps/microstl EXCLUDE_FROM_ALL)
add_subdirectory(deps/nativefiledialog-extended-1.2.1 EXCLUDE_FROM_ALL)
add_subdirectory(deps/stb_image-2.30 EXCLUDE_FROM_ALL)

# Automatically recursively gather all source files under src.
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

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
