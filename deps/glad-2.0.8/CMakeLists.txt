cmake_minimum_required(VERSION 3.30)
project(glad)

add_library(glad)
target_sources(glad PRIVATE src/gl.c)
target_include_directories(glad PUBLIC include)
