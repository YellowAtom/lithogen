cmake_minimum_required(VERSION 3.30)
project(imgui)

add_library(imgui)

target_sources(imgui PRIVATE
		imgui.cpp
		imgui_demo.cpp
		imgui_draw.cpp
		imgui_tables.cpp
		imgui_widgets.cpp
		backends/imgui_impl_glfw.cpp
		backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC .)
target_link_libraries(imgui PUBLIC glfw)
