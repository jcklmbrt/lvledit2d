cmake_minimum_required (VERSION 3.8)

project(lvledit2d)
add_executable(lvledit2d WIN32 )

set(wxBUILD_SHARED OFF)
set(wxUSE_STL ON)
set(wxUSE_STD_CONTAINERS ON)
set(wxWidgets_DIR         "3rdparty/wxWidgets")
set(wxWidgets_LIB_DIR     "3rdparty/wxWidgets/lib")
set(wxWidgets_INCLUDE_DIR "3rdparty/wxWidgets/include")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(${wxWidgets_DIR})
target_include_directories(lvledit2d PUBLIC ${wxWidgets_USE_FILE})
target_link_libraries(lvledit2d wx::net wx::core wx::base wx::gl)

target_include_directories(lvledit2d PUBLIC "3rdparty/glm")

target_include_directories(lvledit2d PUBLIC .)
target_sources(lvledit2d PRIVATE "src/lvledit2d.cpp" "src/lvledit2d.hpp" "src/mainframe.cpp" "src/glcanvas.cpp" )