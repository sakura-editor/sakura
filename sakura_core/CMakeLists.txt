cmake_minimum_required(VERSION 3.0.2)

# define a variable of project name
set(project_name sakura)

# define a project name
project (${project_name})

# set the project as the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${project_name})

# define variables by file GLOB
file(GLOB_RECURSE SOURCE_FILES   ./*.cpp)
file(GLOB_RECURSE HEADER_FILES   ./*.h)
file(GLOB_RECURSE RESOURCE_FILES ./*.rc)

#foreach( srcTemp ${SOURCE_FILES})
#	message("SOURCE_FILES = " ${srcTemp})
#endforeach( srcTemp ${SOURCE_FILES})

# define sources files of an executable
add_executable(${project_name} WIN32 ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

# configure precompiled header
if(MSVC)
	set(PrecompiledSource  "StdAfx.cpp")
	set(PrecompiledHeader  "StdAfx.h")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Yu")
	message("CMAKE_CXX_FLAGS   " ${CMAKE_CXX_FLAGS})
	message("project_name      " ${project_name})
	message("PrecompiledSource " ${PrecompiledSource})
	message("PrecompiledHeader " ${PrecompiledHeader})
	set_source_files_properties(${PrecompiledSource} PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\"")
endif(MSVC)

add_dependencies(${project_name} HeaderMake)

get_target_property(HeaderMake_RUNTIME_OUTPUT_DIRECTORY HeaderMake RUNTIME_OUTPUT_DIRECTORY)
set(update_HeaderMake_BAT      ${CMAKE_CURRENT_SOURCE_DIR}/update-HeaderMake.bat)
set(update_HeaderMake_DIR_EXE  ${HeaderMake_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(update_HeaderMake_DIR_IN   ${CMAKE_CURRENT_SOURCE_DIR})
set(update_HeaderMake_DIR_OUT  ${CMAKE_CURRENT_SOURCE_DIR})

# set pre-Build
add_custom_command(
	TARGET ${project_name}
	PRE_BUILD
	COMMAND ${update_HeaderMake_BAT} ${update_HeaderMake_DIR_EXE} ${update_HeaderMake_DIR_IN} ${update_HeaderMake_DIR_OUT}
)

# set pre-Build
add_custom_command(
	TARGET ${project_name}
	PRE_BUILD
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/update-gitHash.bat ${CMAKE_CURRENT_SOURCE_DIR}
)

include_directories(./)

# add definition
add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-DWIN32 -D_WINDOWS -DNOMINMAX -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0501)

target_link_libraries(${project_name} comctl32.lib Imm32.lib mpr.lib imagehlp.lib Shlwapi.lib)
