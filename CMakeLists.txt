cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)

project(lvledit2d)

set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxUSE_STD_CONTAINERS ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(WIN32)
	add_executable(lvledit2d WIN32)
	set(wxWidgets_DIR         "3rdparty/wxWidgets")
	set(wxWidgets_LIB_DIR     "3rdparty/wxWidgets/lib")
	set(wxWidgets_INCLUDE_DIR "3rdparty/wxWidgets/include")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)

	add_subdirectory(${wxWidgets_DIR})
	target_include_directories(lvledit2d PUBLIC ${wxWidgets_USE_FILE})
	target_link_libraries(lvledit2d wx::core wx::base wx::aui)
else()
	add_executable(lvledit2d)
	find_package(wxWidgets COMPONENTS core base aui)
	if(wxWidgets_FOUND)
	  include(${wxWidgets_USE_FILE})
	  target_link_libraries(lvledit2d ${wxWidgets_LIBRARIES})
	endif()	
endif()

target_include_directories(lvledit2d PUBLIC "3rdparty/glm")
target_include_directories(lvledit2d PUBLIC .)
target_sources(lvledit2d PRIVATE
	"src/lvledit2d.cpp"
	"src/mainframe.cpp"
	"src/toolbar.cpp"
	"src/drawpanel.cpp"
	"src/viewmatrix.cpp"
	"src/convexpolygon.cpp"
	"src/edit/rectangleedit.cpp"
	"src/edit/selectionedit.cpp"
	"src/edit/lineedit.cpp" )