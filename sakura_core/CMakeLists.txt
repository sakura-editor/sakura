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

get_target_property(HeaderMake_RUNTIME_OUTPUT_DIRECTORY HeaderMake RUNTIME_OUTPUT_DIRECTORY)
set(update_HeaderMake_BAT      ${CMAKE_CURRENT_SOURCE_DIR}/../sakura/funccode.bat)
set(update_HeaderMake_DIR_EXE  ${HeaderMake_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(update_HeaderMake_DIR_DAT  ${CMAKE_CURRENT_SOURCE_DIR})
set(update_gitHash_BAT         ${CMAKE_CURRENT_SOURCE_DIR}/../sakura/githash.bat)
set(update_gitHash_DIR_DAT     ${CMAKE_CURRENT_SOURCE_DIR})

# set pre-Build
add_custom_command(
	TARGET ${project_name}
	PRE_BUILD
	COMMAND ${update_HeaderMake_BAT} ${update_HeaderMake_DIR_EXE} ${update_HeaderMake_DIR_DAT}
)

# set pre-Build
add_custom_command(
	TARGET ${project_name}
	PRE_BUILD
	COMMAND ${update_gitHash_BAT} ${update_gitHash_DIR_DAT}
)

include_directories(./)

# add definition
add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-DWIN32 -D_WINDOWS -DNOMINMAX -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0501)

target_link_libraries(${project_name} comctl32.lib Imm32.lib mpr.lib imagehlp.lib Shlwapi.lib)
