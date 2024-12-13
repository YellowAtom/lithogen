cmake_minimum_required(VERSION 3.30)
project(LithoGen)

set(CMAKE_CXX_STANDARD 23)

add_executable(LithoGen src/main.cpp)

set_target_properties(LithoGen PROPERTIES OUTPUT_NAME "lithogen")
