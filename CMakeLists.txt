cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

include(CheckCXXCompilerFlag)

project(Nbody CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_CUDA "Enable CUDA GPU execution" OFF)

if (ENABLE_CUDA)
  	enable_language(CUDA)
else()
	check_cxx_compiler_flag(" -mavx2" CXX_SUPPORTS_AVX2)
	if(CXX_SUPPORTS_AVX2)
	    message(STATUS "Compiler supports AVX2 via -mavx2")
	    add_definitions(-mavx2)
	    add_compile_definitions (ENABLE_AVX)
	else()
	    message(STATUS "Compiler does NOT support AVX2 via -mavx2")
	endif()
endif()

# Set RPATH
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
endif("${isSystemDir}" STREQUAL "-1")

find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)
find_package(TBB REQUIRED)

add_subdirectory(third_party/SDL)
install(TARGETS SDL3-shared)

add_subdirectory(third_party/imgui)
add_subdirectory(src/nbody_system)
add_subdirectory(src/app)


# Install shaders into the run directory
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/app/shaders/"
        DESTINATION "bin/shaders"
        FILES_MATCHING PATTERN "*.glsl"
)