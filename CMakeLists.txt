cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

include(CheckCXXCompilerFlag)

project(Nbody CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_CUDA "Enable CUDA GPU execution" OFF)

set(DEVICE_CXX_FLAGS "")
if (ENABLE_CUDA)
  	enable_language(CUDA)
  	set(CMAKE_CXX_COMPILER "nvc++")
  	string(APPEND DEVICE_CXX_FLAGS " -stdpar=gpu -fast")
else()
	check_cxx_compiler_flag(" -mavx2" CXX_SUPPORTS_AVX2)
	if(CXX_SUPPORTS_AVX2)
	    message(STATUS "Compiler supports AVX2 via -mavx2")
	    add_definitions(-mavx2)
	    add_compile_definitions (ENABLE_AVX)
	else()
	    message(STATUS "Compiler does NOT support AVX2 via -mavx2")
	endif()
  	string(APPEND DEVICE_CXX_FLAGS " -Ofast -march=native")
endif()


if(CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "Adding device specific compiler flags to CMAKE_CXX_FLAGS: '${DEVICE_CXX_FLAGS}'")
  string(APPEND CMAKE_CXX_FLAGS ${DEVICE_CXX_FLAGS})
endif()


add_executable(nbody main.cc src/nbody_system/initial_condition.cc src/nbody_system/system.cc)

target_include_directories(nbody PRIVATE src/nbody_system)

if (ENABLE_CUDA)
  add_compile_definitions (ENABLE_CUDA)
  target_compile_options(nbody PRIVATE -stdpar)
  target_link_options(nbody PRIVATE -stdpar)
else()
  find_package(TBB REQUIRED)
  target_link_libraries(nbody PRIVATE TBB::tbb)
endif()


install(TARGETS nbody)