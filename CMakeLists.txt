# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#

cmake_minimum_required (VERSION 3.8)
project ("WinGui")

add_executable(ExampleApp "Example.cpp" "WinGui.cpp" "WinGui.h" "resource.h" "resource.rc")

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ExampleApp)
set_target_properties(ExampleApp PROPERTIES LINK_FLAGS /SUBSYSTEM:WINDOWS)

target_compile_definitions(ExampleApp PRIVATE
	UNICODE
	_UNICODE
)

file(COPY "resources/icon.ico" DESTINATION "resources/")
