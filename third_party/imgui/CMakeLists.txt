
cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

# List of source files
set(IMGUI_CC_FILES
	imgui.cpp
	imgui_demo.cpp
	imgui_draw.cpp
	imgui_impl_glfw.cpp
	imgui_impl_opengl3.cpp
	imgui_tables.cpp
	imgui_widgets.cpp
)

# List of public header files
set(IMGUI_PUBLIC_HH_FILES
	imgui.h
	imgui_impl_glfw.h
	imgui_impl_opengl3.h
)

add_library(imgui SHARED)

target_sources(imgui PRIVATE ${IMGUI_CC_FILES})
target_sources(imgui PUBLIC FILE_SET HEADERS FILES ${IMGUI_PUBLIC_HH_FILES})

target_link_libraries(imgui PUBLIC GLEW OpenGL glfw)

install(TARGETS imgui FILE_SET HEADERS)
