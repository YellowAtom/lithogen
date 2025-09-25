# Configuration for GLFW.
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Configuration for battery embed.
set(B_PRODUCTION_MODE ON CACHE BOOL "" FORCE)

CPMAddPackage("https://github.com/batterycenter/embed.git#v1.2.19")
CPMAddPackage("https://github.com/glfw/glfw.git#3.4")
CPMAddPackage("https://github.com/g-truc/glm.git#1.0.1")
CPMAddPackage("https://github.com/btzy/nativefiledialog-extended.git#v1.2.1")
CPMAddPackage("https://github.com/Perlmint/glew-cmake.git#a9714943d5e08cbc596a7f2758195148df5dc628")

############################################################
#                         MicroSTL                         #
############################################################

CPMAddPackage(
		NAME microstl
		GIT_REPOSITORY "https://github.com/cry-inc/microstl.git"
		GIT_TAG "ec3868a14d8eff40f7945b39758edf623f609b6f"
		DOWNLOAD_ONLY YES
)

if (microstl_ADDED)
	add_library(microstl INTERFACE EXCLUDE_FROM_ALL)
	target_include_directories(microstl INTERFACE ${microstl_SOURCE_DIR}/include)
endif ()

############################################################
#                          ImGUI                           #
############################################################

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

	add_library(imgui STATIC EXCLUDE_FROM_ALL ${imgui_FINAL_SOURCE_FILES})
	target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR}/include)
	target_link_libraries(imgui PRIVATE glfw)
	set_target_properties(imgui PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${imgui_BINARY_DIR}")
endif ()

############################################################
#                         STB Image                        #
############################################################

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

	add_library(stb_image STATIC EXCLUDE_FROM_ALL ${stb_SOURCE_DIR}/stb_image.c)
	target_include_directories(stb_image PUBLIC ${stb_SOURCE_DIR}/includes)
	set_target_properties(stb_image PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "${stb_BINARY_DIR}")
endif ()