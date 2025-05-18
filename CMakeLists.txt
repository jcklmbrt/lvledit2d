cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project(lvledit2d)

set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxUSE_STD_CONTAINERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLAD_SOURCES_DIR "${PROJECT_SOURCE_DIR}/3rdparty/glad")
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)
add_subdirectory("3rdparty/glfw" EXCLUDE_FROM_ALL)

glad_add_library(glad_gl_core_33 REPRODUCIBLE LOADER API gl:core=3.3)

add_executable(lvledit2d)
target_link_libraries(lvledit2d glfw glad_gl_core_33)

target_include_directories(lvledit2d PUBLIC 
	"3rdparty/glm"
	"3rdparty/stb"
	"3rdparty/glfw/include"
	".")

target_sources(lvledit2d PRIVATE
	"src/main.cpp"
	"src/geometry.cpp"
	"src/edit/editorcontext.cpp"
	"src/gl/glcontext.cpp"
	"src/gl/texture.cpp"
	"src/edit/l2dfile.cpp" 
)
