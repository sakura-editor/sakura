cmake_minimum_required(VERSION 3.0.2)

# define a variable of project name
set(project_name sakura)

# define a project name
project (${project_name} LANGUAGES CXX)

# set the project as the startup project
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT ${project_name})

# define variables by file GLOB
file(GLOB_RECURSE SOURCE_FILES   ./*.cpp)
file(GLOB_RECURSE HEADER_FILES   ./*.h)
file(GLOB_RECURSE RESOURCE_FILES ./*.rc)

# define sources files of an executable
add_executable(${project_name} WIN32 ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

# configure precompiled header
if(MSVC)
	set(PrecompiledSource  "StdAfx.cpp")
	set(PrecompiledHeader  "StdAfx.h")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Yu")
	set_source_files_properties(${PrecompiledSource} PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\"")
endif(MSVC)

# use static version of c-runtime instead of DLL version.
if(MSVC)
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
endif(MSVC)

add_dependencies(${project_name} HeaderMake)
add_dependencies(${project_name} OutHeaderMake)
add_dependencies(${project_name} OutGitHash)

include_directories(./)

# add definition
add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-DWIN32 -D_WINDOWS -DNOMINMAX -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0501)

target_link_libraries(${project_name} comctl32.lib Imm32.lib mpr.lib imagehlp.lib Shlwapi.lib)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	set(CMAKE_EXE_LINKER_FLAGS "-static -mwindows")
endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
