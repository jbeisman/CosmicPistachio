cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

add_executable(nbody app.cc camera.cc renderer.cc color_palette.cc)
target_link_libraries(nbody PRIVATE SDL3::SDL3 GLEW OpenGL imgui system)

if (ENABLE_CUDA)
  add_compile_definitions (ENABLE_CUDA)
  target_compile_options(nbody PRIVATE -stdpar=gpu)
  target_link_options(nbody PRIVATE -stdpar)
else()
  find_package(TBB REQUIRED)
  target_link_libraries(nbody PRIVATE TBB::tbb)
endif()

install(TARGETS nbody)
