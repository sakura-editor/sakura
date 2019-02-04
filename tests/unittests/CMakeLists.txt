cmake_minimum_required(VERSION 3.6)
include(${CMAKE_SOURCE_DIR}/runtime.cmake)

# define a variable of project name
set(project_name tests1)

# define a variable SRC with file GLOB
file(GLOB TEST_SRC ${CMAKE_CURRENT_LIST_DIR}/test*.cpp)
file(GLOB TARGET_SRC ${CMAKE_CURRENT_LIST_DIR}/code*.cpp)

# define sources files of an executable
add_executable(${project_name} ${TEST_SRC} ${TARGET_SRC})

# add definitions
target_compile_definitions(${project_name} PUBLIC _CONSOLE UNICODE _UNICODE)

# add include directories for sakura_core
target_include_directories(${project_name} PRIVATE ${CMAKE_SOURCE_DIR}/../sakura_core)

# use shared library version
# this is required when using parameterized test
if (BUILD_SHARED_LIBS)
	# fix linker error "class testing::internal::Mutex testing::internal::g_linked_ptr_mutex"
	target_compile_definitions(${project_name} PUBLIC -DGTEST_LINKED_AS_SHARED_LIBRARY=1)

	# suppress warning
	if (MSVC)
		target_compile_options(${project_name} PUBLIC /wd4251 PUBLIC /wd4275)
	endif(MSVC)
endif (BUILD_SHARED_LIBS)

# link libraries
target_link_libraries(${project_name} PRIVATE gtest)
target_link_libraries(${project_name} PRIVATE gtest_main)

# Hacks to reuse compiled editor objects.
target_compile_definitions(${project_name} PRIVATE WIN32 _WIN32_WINNT=_WIN32_WINNT_WIN7)
if (MSVC)
	target_compile_options (${project_name} PRIVATE $<$<CONFIG:Release>:/GL> /MT$<$<CONFIG:Debug>:d> /GF /FD /EHsc /Zi /TP /source-charset:utf-8 /execution-charset:shift_jis)
	target_link_libraries  (${project_name} PRIVATE $<$<CONFIG:Release>:-LTCG> "${CMAKE_CURRENT_LIST_DIR}/../../sakura/${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>/*.obj")
	set_target_properties  (${project_name} PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>"
	)
elseif (MINGW)
	target_compile_options (${project_name} PRIVATE -finput-charset=utf-8 -fexec-charset=cp932)
	target_compile_definitions (${project_name} PUBLIC $<$<CONFIG:Debug>:_DEBUG>)
	file (GLOB_RECURSE ALL_O
		"${CMAKE_CURRENT_LIST_DIR}/../../sakura_core/*.cpp"
		"${CMAKE_CURRENT_LIST_DIR}/../../sakura_core/*.rc"
	)
	list (TRANSFORM ALL_O REPLACE "\\.(cpp|rc)$" ".o")
	target_link_libraries (${project_name} PRIVATE ${ALL_O})
endif ()
target_link_libraries (${project_name} PRIVATE winspool ole32 oleaut32 uuid comctl32 imm32 mpr imagehlp shlwapi winmm windowscodecs msimg32)
