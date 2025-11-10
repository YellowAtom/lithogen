cmake_minimum_required(VERSION 3.30)
project(microstl LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(microstl INTERFACE)
target_include_directories(microstl INTERFACE include)