cmake_minimum_required(VERSION 2.6.4)

# define a variable of project name
set(project_name sakura_lang_en_US)

# define a project name
project (${project_name})

include_directories(./)

# define variables by file GLOB
file(GLOB_RECURSE SOURCE_FILES   ./*.cpp)
file(GLOB_RECURSE HEADER_FILES   ./*.h)
file(GLOB_RECURSE RESOURCE_FILES ./*.rc)

#foreach( srcTemp ${SOURCE_FILES})
#	message("SOURCE_FILES = " ${srcTemp})
#endforeach( srcTemp ${SOURCE_FILES})

# define sources files of an executable
# 'MODULE' libraries  are not linked directly.
# see https://cmake.org/cmake/help/v3.0/command/add_library.html
add_library(${project_name} MODULE ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

add_dependencies(${project_name} HeaderMake)
add_dependencies(${project_name} OutHeaderMake)
add_dependencies(${project_name} OutGitHash)

get_target_property(HeaderMake_RUNTIME_OUTPUT_DIRECTORY HeaderMake RUNTIME_OUTPUT_DIRECTORY)
set(update_HeaderMake_BAT      ${sakura_SOURCE_DIR}/../sakura/funccode.bat)
set(update_HeaderMake_DIR_EXE  ${HeaderMake_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(update_HeaderMake_DIR_DAT  ${sakura_SOURCE_DIR})
set(update_gitHash_BAT         ${sakura_SOURCE_DIR}/../sakura/githash.bat)
set(update_gitHash_DIR_DAT     ${sakura_SOURCE_DIR})

# add definition
add_definitions(-D_UNICODE -DUNICODE)
add_definitions(-DWIN32 -D_WINDOWS -DNOMINMAX -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0501)

if(MSVC)
	# avoid error LNK2001 for "__DllMainCRTStartup@12"
	set_target_properties(${project_name} PROPERTIES LINK_FLAGS "/NOENTRY")
endif(MSVC)

# fix the error 'Missing variable is: CMAKE_RC_CREATE_SHARED_LIBRARY'
# https://cmake.org/pipermail/cmake/2012-January/048647.html
set_target_properties(${project_name} PROPERTIES LINKER_LANGUAGE "CXX" )

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
	# https://cmake.org/cmake/help/v3.0/prop_tgt/PREFIX.html?highlight=prefix
	set_target_properties(${project_name} PROPERTIES PREFIX "" )
endif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
