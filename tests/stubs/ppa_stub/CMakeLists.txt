cmake_minimum_required(VERSION 3.13)

# define a variable of project name
set(project_name ppa_stub)

# define a project name
project (${project_name} LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)    # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...

# define variables by file GLOB
file(GLOB_RECURSE SOURCES *.cpp)

# define sources files of an executable
add_library(${project_name} MODULE ${SOURCES})

# fix the error 'Missing variable is: CMAKE_RC_CREATE_SHARED_LIBRARY'
# https://cmake.org/pipermail/cmake/2012-January/048647.html
set_target_properties(${project_name}
  PROPERTIES
    LINKER_LANGUAGE "CXX"
  )

if(MINGW)
    # avoid prefixing of DLL name, set PREFIX to blank.
	# https://cmake.org/cmake/help/v3.12/prop_tgt/PREFIX.html?highlight=prefix
	set_target_properties(${project_name}
	  PROPERTIES
	    PREFIX		""
    )
endif()
