cmake_minimum_required(VERSION 3.23 FATAL_ERROR)

# List of source files
set(SYS_CC_FILES
	initial_condition.cc
	system.cc
)

# List of public header files
set(SYS_PUBLIC_HH_FILES
	system.hh
)

add_library(system SHARED)

target_sources(system PRIVATE ${SYS_CC_FILES})
target_sources(system PUBLIC FILE_SET HEADERS FILES ${SYS_PUBLIC_HH_FILES})

if (ENABLE_CUDA)
  add_compile_definitions (ENABLE_CUDA)
  target_compile_options(system PRIVATE -stdpar=gpu)
  target_link_options(system PRIVATE -stdpar)
else()
  find_package(TBB REQUIRED)
  target_link_libraries(system PRIVATE TBB::tbb)
endif()

install(TARGETS system FILE_SET HEADERS)
