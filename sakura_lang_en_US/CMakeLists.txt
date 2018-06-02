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
add_library(${project_name} SHARED ${SOURCE_FILES} ${HEADER_FILES} ${RESOURCE_FILES})

# add definition
add_definitions(-DWIN32 -D_WINDOWS -DNOMINMAX -DWINVER=0x0500 -D_WIN32_WINNT=0x0500 -D_WIN32_IE=0x0501)

# avoid error LNK2001 for "__DllMainCRTStartup@12"
set_target_properties(${project_name} PROPERTIES LINK_FLAGS "/NOENTRY")
