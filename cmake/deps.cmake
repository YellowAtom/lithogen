# Configuration for GLFW.
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Configuration for battery embed.
set(B_PRODUCTION_MODE ON CACHE BOOL "" FORCE)

CPMAddPackage("https://github.com/batterycenter/embed.git@1.2.19#fdbae3fa33e96824726b68d9a8f64ba6be3833c6")
CPMAddPackage("https://github.com/glfw/glfw.git@3.4#7b6aead9fb88b3623e3b3725ebb42670cbe4c579")
CPMAddPackage("https://github.com/g-truc/glm.git@1.0.1#0af55ccecd98d4e5a8d1fad7de25ba429d60e863")
CPMAddPackage("https://github.com/btzy/nativefiledialog-extended.git@1.2.1#86d5f2005fe1c00747348a12070fec493ea2407e")

############################################################
#                           Glad                           #
############################################################

CPMAddPackage(
		NAME glad
		VERSION 2.0.8
		GIT_REPOSITORY "https://github.com/Dav1dde/glad.git"
		GIT_TAG "73db193f853e2ee079bf3ca8a64aa2eaf6459043"
		DOWNLOAD_ONLY YES
)

if (glad_ADDED)
	set(GLAD_SOURCES_DIR ${glad_SOURCE_DIR})

	add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)

	glad_add_library(glad STATIC EXCLUDE_FROM_ALL REPRODUCIBLE LOCATION "${glad_BINARY_DIR}/source" API gl:core=4.6)
	set_target_properties(glad PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${glad_BINARY_DIR})
endif ()

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
		VERSION 1.92.3
		GIT_REPOSITORY "https://github.com/ocornut/imgui.git"
		GIT_TAG "bf75bfec48fc00f532af8926130b70c0e26eb099"
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