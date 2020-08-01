cmake_minimum_required(VERSION 3.12)

set(CMAKE_CXX_STANDARD 17)          # C++17...
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++17 is required...
set(CMAKE_CXX_EXTENSIONS OFF)		    # -std=c++17

project (SakuraEditor LANGUAGES CXX)

set(PLATFORM Win32)
if (MINGW)
  set(PLATFORM MinGW)
endif (MINGW)

option(BUILD_GTEST "Build GoogleTest." ON)

if (NOT BUILD_BASE_DIR)
  set(BUILD_BASE_DIR ${CMAKE_BINARY_DIR})
endif (NOT BUILD_BASE_DIR)

if (NOT GOOGLETEST_INSTALL_DIR)
  set(GOOGLETEST_INSTALL_DIR ${BUILD_BASE_DIR}/${PLATFORM}/tools/googletest)
endif (NOT GOOGLETEST_INSTALL_DIR)

# add generic preprocessor symbol definitions
add_compile_definitions(
  $<IF:$<CONFIG:Debug>,_DEBUG,NDEBUG>
  _UNICODE
  UNICODE
   _WIN32_WINNT=_WIN32_WINNT_WIN7
)

if (MSVC)
  # add generic compile options of MSVC
	foreach(flag_var CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    # use static version of c-runtime instead of DLL version.
		if(${flag_var} MATCHES "/MD")
			string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
		endif(${flag_var} MATCHES "/MD")
	endforeach(flag_var)
endif (MSVC)

if (MINGW)
  # add generic compile options for GCC
  add_compile_options(
    -finput-charset=utf-8
    -fexec-charset=cp932
  )
  set(CMAKE_RC_FLAGS "-c utf8 --language=0411")
  set(CMAKE_EXE_LINKER_FLAGS "-static")
endif (MINGW)

function(run_compiletests compiletests_config)
  set(compiletests_run_dir ${BUILD_BASE_DIR}/${PLATFORM}/${compiletests_config}/tests/compiletests)
  file(MAKE_DIRECTORY ${compiletests_run_dir})
  if (NOT EXISTS ${compiletests_run_dir}/CMakeCache.txt)
    execute_process(
      COMMAND "${CMAKE_CURRENT_SOURCE_DIR}/tests/compiletests.run.cmd"
        ${PLATFORM}
        ${compiletests_config}
      WORKING_DIRECTORY ${compiletests_run_dir}
    )
  endif (NOT EXISTS ${compiletests_run_dir}/CMakeCache.txt)
endfunction(run_compiletests)

run_compiletests(${CMAKE_BUILD_TYPE})

add_subdirectory(HeaderMake)
add_subdirectory(sakura_core)
add_subdirectory(sakura_lang_en_US)

enable_testing()
add_subdirectory(tests)
