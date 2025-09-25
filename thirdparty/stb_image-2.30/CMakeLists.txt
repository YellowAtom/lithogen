cmake_minimum_required(VERSION 3.30)
project(stb_image)

add_library(stb_image)
target_sources(stb_image PRIVATE stb_image.c)
target_include_directories(stb_image PUBLIC .)