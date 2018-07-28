cmake_minimum_required(VERSION 3.6)
include(${CMAKE_SOURCE_DIR}/runtime.cmake)

# define a variable of project name
set(project_name tests1)

# define a variable SRC with file GLOB
file(GLOB SRC ${CMAKE_CURRENT_LIST_DIR}/test*.cpp)

# add include directories
include_directories(${googletest_SOURCE_DIR}/include)

# define sources files of an executable
add_executable(${project_name} ${SRC})

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
target_link_libraries(${project_name} gtest)
target_link_libraries(${project_name} gtest_main)
