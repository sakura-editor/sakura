cmake_minimum_required(VERSION 3.12)

# define a variable of project name
set(project_name sakura_lang_en_US)
project (${project_name} LANGUAGES CXX)

# define variables by file GLOB
file(GLOB_RECURSE RESOURCE_SCRIPTS *.rc)

# define sources files of an executable
add_library(${project_name} MODULE ${RESOURCE_SCRIPTS})

# set include directories
target_include_directories(${project_name}
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../sakura_core
  PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/../sakura_core
)

# add dependencies for build
add_dependencies(${project_name} SourceCodeHash FunccodeForResource)

if(MSVC)
	# avoid error LNK2001 for "__DllMainCRTStartup@12"
	set_target_properties(${project_name}
    PROPERTIES
      LINK_FLAGS "/NOENTRY"
  )
endif(MSVC)

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
