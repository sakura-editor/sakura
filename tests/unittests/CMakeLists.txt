cmake_minimum_required(VERSION 3.12)

# define a variable of project name
set(project_name tests1)
project(${project_name} LANGUAGES CXX)

# define a variable SRC with file GLOB
file(GLOB TEST_SRC ${CMAKE_CURRENT_LIST_DIR}/test*.cpp)
file(GLOB TARGET_SRC ${CMAKE_CURRENT_LIST_DIR}/code*.cpp)

# define sources files of an executable
add_executable(${project_name} ${TEST_SRC} ${TARGET_SRC})

# add preprocessor symbol definitions
target_compile_definitions(${project_name} PUBLIC CONSOLE)

# link libraries
target_link_libraries(${project_name}
  PRIVATE
    sakura_core
	comctl32
	imagehlp
	shlwapi
	uuid
	ole32
	oleaut32
    winspool
	winmm
	windowscodecs
	msimg32
	imm32
	mpr
    GTest::GTest
	GTest::Main
)

# add compiler-specific linker options
if (MSVC)
  target_link_libraries(${project_name}
    PRIVATE
	  $<$<CONFIG:Release>:-LTCG>
  )
  set_target_properties(${project_name}
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_GENERATOR_PLATFORM}/$<CONFIG>"
  )
elseif (MINGW)
endif ()

# enable testing
enable_testing()

# add test
add_test(NAME unitTest1
  COMMAND $<TARGET_FILE:${project_name}>
    --gtest_output=xml:$<TARGET_FILE:${project_name}>-googletest-${PLATFORM}-${CMAKE_BUILD_TYPE}.xml
)
