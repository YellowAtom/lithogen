
cmake_minimum_required(VERSION 3.30)

project(LithoGen VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON) # Refuses to compile on older versions of CXX.

set(SHARED_SOURCE_FILES src/main.cpp)

# Create an executable out of the provided source files.
add_executable(LithoGen ${SHARED_SOURCE_FILES})

# Manually set the file name of the executable.
set_target_properties(LithoGen PROPERTIES OUTPUT_NAME "lithogen")
