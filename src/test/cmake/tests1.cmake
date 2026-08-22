# CMake script for tests1
#
# requires
#   ${7ZIP_EXECUTABLE}
#   ${ARCH}
#   ${CMAKE_GENERATOR_PLATFORM}
#   ${EXE_ARCH}

# Create a custom command for tests1.exe.manifest generation
add_custom_command(
  OUTPUT "${CMAKE_BINARY_DIR}/tests1.exe.manifest"
  COMMAND ${CMAKE_COMMAND} 
    -DSOURCE_DIR=${CMAKE_SOURCE_DIR}
    -DEXE_NAME="tests1.exe"
    -DEXE_ARCH="${EXE_ARCH}"
    -DOUTPUT_FILE="${CMAKE_BINARY_DIR}/tests1.exe.manifest"
    -P ${CMAKE_SOURCE_DIR}/src/main/cmake/manifest.cmake
  COMMENT "Generating tests1.exe.manifest"
)

# Create a custom target that depends on the generated file
add_custom_target(generate_tests1_exe_manifest
  DEPENDS
    "${CMAKE_BINARY_DIR}/tests1.exe.manifest"
)

# Find GoogleTest's package(required)
find_package(GTest CONFIG REQUIRED)

# Find OpenCppCoverage for coverage test
find_program(OpenCppCoverage_EXECUTABLE OpenCppCoverage
  PATHS
    "$ENV{ProgramFiles}/OpenCppCoverage"
)

if(OpenCppCoverage_EXECUTABLE)
  message(STATUS "Found OpenCppCoverage: ${OpenCppCoverage_EXECUTABLE}")
endif()

find_program(UV_EXECUTABLE
  NAMES uv
  REQUIRED
)

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
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.h
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.hpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.h
)

# define source files of tests1
file(GLOB_RECURSE TESTS1_SOURCES
  ${CMAKE_SOURCE_DIR}/src/test/cpp/tests1/*.cpp
  ${CMAKE_SOURCE_DIR}/src/test/cpp/*.cpp
  ${CMAKE_SOURCE_DIR}/src/test/resources/*.cpp
)

if(MSVC)
  # coverage.cppのみC++17準拠にする
  set_source_files_properties(
    ${CMAKE_SOURCE_DIR}/src/test/resources/coverage.cpp
    PROPERTIES
      COMPILE_FLAGS "/std:c++17"
      SKIP_PRECOMPILE_HEADERS ON
  )
endif(MSVC)

if(MINGW)
  # coverage.cppをリストから削除
  list(REMOVE_ITEM TESTS1_SOURCES ${CMAKE_SOURCE_DIR}/src/test/resources/coverage.cpp)
endif(MINGW)

# define resource files of tests1
set(TESTS1_RESOURCE_SCRIPTS ${CMAKE_SOURCE_DIR}/sakura_core/tests1_rc.rc)

set(TEST_DLLPLUGIN_DIR "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-dllplugin")
set(TEST_DLLPLUGIN_TARGET dll_plugin1)
set(TESTS1_RESOURCE_STAGE_DIR "${CMAKE_BINARY_DIR}/tests1_resources")
set(TEST_OUTLINE_STAGE_DIR "${TESTS1_RESOURCE_STAGE_DIR}/outline")

if(MINGW)
  # Convert RC files to UTF-8 for MinGW
  convert_rc_files_to_utf8(TESTS1_RESOURCE_SCRIPTS "ja-JP" ${CMAKE_BINARY_DIR})
endif()

# Create a custom target for test_resource_zip generation
add_custom_target(test_resource_zip
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${TESTS1_RESOURCE_STAGE_DIR}/test-plugin"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${TESTS1_RESOURCE_STAGE_DIR}/test-plugin"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-plugin
    ${TESTS1_RESOURCE_STAGE_DIR}/test-plugin
  COMMAND ${7ZIP_EXECUTABLE}
    u -tzip -r -mcu=on
    ${CMAKE_BINARY_DIR}/resources.ja-JP.zip
    ${TESTS1_RESOURCE_STAGE_DIR}/test-plugin
    > NUL
  BYPRODUCTS ${CMAKE_BINARY_DIR}/resources.ja-JP.zip
  COMMENT "Generating resources.ja-JP.zip"
)

# Create a custom target for outline.zip generation
add_custom_target(test_outline_zip
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${TEST_OUTLINE_STAGE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${TEST_OUTLINE_STAGE_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Asm.asm.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.asm"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/VisualBasic.bas.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.bas"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/VisualBasic.cls.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.cls"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Cobol.cbl.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.cbl"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Cpp.cpp.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.cpp"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/DosBatch.bat.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.bat"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Awk.awk.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.awk"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Csv.csv.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.csv"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Html.html.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.html"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Java.java.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.java"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Python.py.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.py"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/OraclePLSQL.sql.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.sql"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Tex.tex.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.tex"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Perl.pl.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.pl"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/Text.txt.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.txt"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/RuleFile.txt.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.cr1"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/RuleFile.rule.txt"
    "${TEST_OUTLINE_STAGE_DIR}/rule.rule"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/RuleFileRegex.txt.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.cr2"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/RuleFileRegex.rule.txt"
    "${TEST_OUTLINE_STAGE_DIR}/rule_regex.rule"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/RuleFileRegexReplace.txt.txt"
    "${TEST_OUTLINE_STAGE_DIR}/test_source.cr3"
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/src/test/resources/tests1/test-outline/RuleFileRegexReplace.rule.txt"
    "${TEST_OUTLINE_STAGE_DIR}/rule_regex_replace.rule"
  COMMAND ${CMAKE_COMMAND} -E remove -f "${CMAKE_BINARY_DIR}/outline.zip"
  COMMAND ${7ZIP_EXECUTABLE}
    u -tzip -r -mcu=on
    ${CMAKE_BINARY_DIR}/outline.zip
    ${TEST_OUTLINE_STAGE_DIR}
    > NUL
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${TEST_OUTLINE_STAGE_DIR}"
  BYPRODUCTS ${CMAKE_BINARY_DIR}/outline.zip
  COMMENT "Generating outline.zip"
)

# Create a custom target for test_dllplugin_zip generation
add_custom_target(test_dllplugin_zip
  COMMAND ${CMAKE_COMMAND} -E remove_directory "${TESTS1_RESOURCE_STAGE_DIR}/test-dllplugin"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${TESTS1_RESOURCE_STAGE_DIR}/test-dllplugin"
  COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${TEST_DLLPLUGIN_DIR}
    ${TESTS1_RESOURCE_STAGE_DIR}/test-dllplugin
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${OUTPUT_DIRECTORY}/dll_plugin1.dll"
    ${TESTS1_RESOURCE_STAGE_DIR}/test-dllplugin/dll_plugin1.dll
  COMMAND ${7ZIP_EXECUTABLE}
    u -tzip -r -mcu=on
    ${CMAKE_BINARY_DIR}/resources-dllplugin.zip
    ${TESTS1_RESOURCE_STAGE_DIR}/test-dllplugin
    > NUL
  BYPRODUCTS ${CMAKE_BINARY_DIR}/resources-dllplugin.zip
  DEPENDS ${TEST_DLLPLUGIN_TARGET}
  COMMENT "Generating resources-dllplugin.zip"
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
    GTest::gtest
    GTest::gmock
)

set_target_properties(tests1
  PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
    ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}"
)

add_custom_command(TARGET tests1 PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E remove -f $<TARGET_FILE:tests1>
)

if(MINGW)
  set(TESTS1_EXE_MANIFEST "${CMAKE_BINARY_DIR}/tests1.exe.manifest")
  set(TESTS1_MANIFEST_RC "${CMAKE_BINARY_DIR}/tests1_manifest.rc")

  # Create a custom command for tests1_manifest.rc generation
  add_custom_command(
    OUTPUT "${TESTS1_MANIFEST_RC}"
    COMMAND ${CMAKE_COMMAND} 
      -DSOURCE_DIR="${CMAKE_SOURCE_DIR}"
      -DOUTPUT_FILE="${TESTS1_MANIFEST_RC}"
      -DMANIFEST_FILE="${TESTS1_EXE_MANIFEST}"
      -P ${CMAKE_SOURCE_DIR}/src/main/cmake/manifest_resource.cmake
    COMMENT "Generating tests1_manifest.rc"
  )

  target_sources(tests1
    PRIVATE
      "${TESTS1_MANIFEST_RC}"
  )

  # Add include directories for tests1
  target_include_directories(tests1
    PRIVATE
      "$<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/tests1_ja-JP>"
      "$<BUILD_INTERFACE:${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/share/cmigemo>"
  )
  target_link_options(tests1
    PRIVATE
      -mconsole
  )
endif(MINGW)

# Add dependencies
add_dependencies(tests1
  sakura
  sakura_lang_en_US
  sakura_lang_zh_CN
  generate_tests1_exe_manifest
  test_resource_zip
  test_outline_zip
  test_dllplugin_zip
  generate_miniz
  ppa_stub
)
