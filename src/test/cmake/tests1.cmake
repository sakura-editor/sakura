# CMake script for tests1
#
# requires
#   ${7ZIP_EXECUTABLE}
#   ${ARCH}
#   ${CMAKE_GENERATOR_PLATFORM}

# Include GoogleTest's targets
include(${CMAKE_SOURCE_DIR}/src/test/cmake/GoogleTest.cmake)

set(MINIZ_SOURCE_DIR "${CMAKE_SOURCE_DIR}/externals/miniz-cpp")
set(MINIZ_INCLUDE_DIR "${CMAKE_BINARY_DIR}/include/miniz-cpp")

add_custom_command(
  OUTPUT "${MINIZ_SOURCE_DIR}/.git"
  COMMAND ${CMAKE_COMMAND}
    -DGIT_EXECUTABLE:FILEPATH=${GIT_EXECUTABLE}
    -DREPO_ROOT:PATH=${CMAKE_SOURCE_DIR}
    -DSUBMODULE_PATH:STRING=externals/miniz-cpp
    -DLOCK_PATH:FILEPATH=${CMAKE_BINARY_DIR}/cmake-submodule-update.lock
    -P ${CMAKE_SOURCE_DIR}/src/main/cmake/git_submodule_update_locked.cmake
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  COMMENT "Fetching miniz-cpp's source files"
)

add_custom_command(
  OUTPUT "${MINIZ_INCLUDE_DIR}/zip_file.hpp"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${MINIZ_INCLUDE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different "${MINIZ_SOURCE_DIR}/zip_file.hpp" "${MINIZ_INCLUDE_DIR}/zip_file.hpp"
  DEPENDS "${MINIZ_SOURCE_DIR}/.git"
  COMMENT "Copying miniz-cpp/zip_file.hpp to include directory"
)

add_custom_target(generate_miniz ALL
  DEPENDS
    "${MINIZ_INCLUDE_DIR}/zip_file.hpp"
)

# define precompiled headers
set(TESTS1_PCH_HEADER ${CMAKE_SOURCE_DIR}/src/test/resources/pch.h)

# define header files of tests1
file(GLOB_RECURSE TESTS1_HEADERS
  ${CMAKE_SOURCE_DIR}/src/test/cpp/tests1/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/cpp/tests1/*.h
  ${CMAKE_SOURCE_DIR}/src/test/resources/tests1/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/tests1/*.h
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.h
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.h
)

# define source files of tests1
file(GLOB_RECURSE TESTS1_SOURCES
  ${CMAKE_SOURCE_DIR}/src/test/cpp/tests1/*.cpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/tests1/*.cpp
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.cpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.cpp
)

if(MINGW)
  # coverage.cppをリストから削除
  list(REMOVE_ITEM TESTS1_SOURCES ${CMAKE_SOURCE_DIR}/src/test/resources/coverage.cpp)
endif(MINGW)

# define resource files of tests1
set(TESTS1_RESOURCE_SCRIPTS ${CMAKE_SOURCE_DIR}/sakura_core/tests1_rc.rc)

if(MINGW)
  # Convert RC files to UTF-8 for MinGW
  convert_rc_files_to_utf8(TESTS1_RESOURCE_SCRIPTS "ja-JP" ${CMAKE_BINARY_DIR})
endif(MINGW)

# Create a custom target for test_resource_zip generation
add_custom_target(test_resource_zip
  COMMAND ${7ZIP_EXECUTABLE}
    u -tzip -r -mcu=on
    ${CMAKE_BINARY_DIR}/resources.ja-JP.zip
    ${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-plugin
    > NUL
  BYPRODUCTS ${CMAKE_BINARY_DIR}/resources.ja-JP.zip
  COMMENT "Generating resources.ja-JP.zip"
)

# define executable
add_executable(tests1
  ${TESTS1_PCH_HEADER}
  ${TESTS1_HEADERS}
  ${TESTS1_SOURCES}
  ${TESTS1_RESOURCE_SCRIPTS}
)

# Enable precompiled headers
target_precompile_headers(tests1 PRIVATE ${TESTS1_PCH_HEADER})

# add definitions for project
target_compile_definitions(tests1
  PRIVATE
    _CONSOLE
    _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
)

# add include directories for project
target_include_directories(tests1
  PRIVATE
    ${CMAKE_SOURCE_DIR}/src/test/cpp/tests1
    ${CMAKE_SOURCE_DIR}/src/test/resources/tests1
    ${CMAKE_SOURCE_DIR}/src/test/cpp
    ${CMAKE_SOURCE_DIR}/src/test/resources
)

# link libraries
target_link_libraries(tests1
  PRIVATE
    sakura_core
    gmock
    gtest
)

set_target_properties(tests1
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
)

if(MINGW)

  # Add include directories for tests1
  target_include_directories(tests1
    PRIVATE
      "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/tests1_ja-JP>"
  )
  target_link_options(tests1
    PRIVATE
      -mconsole
  )
endif(MINGW)

# Add dependencies
add_dependencies(tests1
  test_resource_zip
  generate_gtest
  generate_miniz
)
