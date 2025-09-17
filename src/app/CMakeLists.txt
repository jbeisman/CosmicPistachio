cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

add_executable(nbody app.cc)


##target_include_directories(nbody PRIVATE src/nbody_system)
##target_include_directories(nbody PRIVATE third_party/imgui)

target_link_libraries(nbody PRIVATE GLEW OpenGL glfw imgui system)

if (ENABLE_CUDA)
  add_compile_definitions (ENABLE_CUDA)
  target_compile_options(nbody PRIVATE -stdpar)
  target_link_options(nbody PRIVATE -stdpar)
else()
  find_package(TBB REQUIRED)
  target_link_libraries(nbody PRIVATE TBB::tbb)
endif()

install(TARGETS nbody)
