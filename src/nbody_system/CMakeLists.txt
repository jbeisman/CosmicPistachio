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

install(TARGETS system FILE_SET HEADERS)
