cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project(lvledit2d)

set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxUSE_STD_CONTAINERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(GLAD_SOURCES_DIR "${PROJECT_SOURCE_DIR}/3rdparty/glad")
add_subdirectory("${GLAD_SOURCES_DIR}/cmake" glad_cmake)
glad_add_library(glad_gl_core_33 REPRODUCIBLE LOADER API gl:core=3.3)

if(WIN32)
	add_executable(lvledit2d WIN32)
	set(wxWidgets_DIR         "3rdparty/wxWidgets")
	set(wxWidgets_LIB_DIR     "3rdparty/wxWidgets/lib")
	set(wxWidgets_INCLUDE_DIR "3rdparty/wxWidgets/include")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	add_subdirectory(${wxWidgets_DIR})
	target_include_directories(lvledit2d PUBLIC ${wxWidgets_USE_FILE})
	target_link_libraries(lvledit2d wx::core wx::base wx::aui wx::gl glad_gl_core_33)
else()
	add_executable(lvledit2d)
	find_package(wxWidgets COMPONENTS core base aui gl)
	if(wxWidgets_FOUND)
	  include(${wxWidgets_USE_FILE})
	  target_link_libraries(lvledit2d ${wxWidgets_LIBRARIES} glad_gl_core_33)
	endif()
endif()

target_include_directories(lvledit2d PUBLIC "3rdparty/glm" ".")

target_sources(lvledit2d PRIVATE
	"src/lvledit2d.cpp"
	"src/mainframe.cpp"
	"src/toolbar.cpp"
	"src/viewmatrix.cpp"
	"src/geometry.cpp"
	"src/edit/rectangleedit.cpp"
	"src/edit/selectionedit.cpp"
	"src/edit/lineedit.cpp"
	"src/edit/editorcontext.cpp"
	"src/historylist.cpp"
    "src/gl/glcanvas.cpp"
	"src/gl/glcontext.cpp"
	"src/notebook.cpp"
	"src/gl/glbackgroundgrid.cpp"
	"src/gl/glsolidgeometry.cpp"
	"src/gl/gltexturegeometry.cpp" 
	"src/edit/textureedit.cpp" 
	"src/texturepanel.cpp" 
	"src/gl/texture.cpp"
	"src/layerpanel.cpp"
	"src/edit/editorlayer.cpp"
	"src/edit/entityedit.cpp"
)
