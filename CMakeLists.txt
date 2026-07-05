cmake_minimum_required(VERSION 3.16)

# Define primary project
project(sakura LANGUAGES CXX)

# Include sakura.cmake for sub-targets
include(${CMAKE_SOURCE_DIR}/src/main/cmake/sakura.cmake)

# Create main executable
add_executable(${PROJECT_NAME} WIN32 ${CMAKE_SOURCE_DIR}/sakura_core/StdAfx.cpp)

# link libraries
target_link_libraries(${PROJECT_NAME}
  PRIVATE
    sakura_core
)

add_custom_command(TARGET ${PROJECT_NAME} PRE_LINK
  COMMAND ${CMAKE_COMMAND} -E remove -f $<TARGET_FILE:${PROJECT_NAME}>
)

# Create language DLL projects
# en-US: 1033 (0x0409) - English (United States)
# zh-CN: 2052 (0x0804) - Chinese (Simplified, PRC)
create_language_dll(en-US 1033)
create_language_dll(zh-CN 2052)

# Include bregonig.cmake for bregonig.dll
include(${CMAKE_SOURCE_DIR}/src/main/cmake/bregonig.cmake)

# Include cmigemo.cmake for migemo.dll
include(${CMAKE_SOURCE_DIR}/src/main/cmake/cmigemo.cmake)

# Include ctags.cmake for ctags command
include(${CMAKE_SOURCE_DIR}/src/main/cmake/ctags.cmake)

# Include diffutils.cmake for diff command
include(${CMAKE_SOURCE_DIR}/src/main/cmake/diffutils.cmake)

# Enable ctest command
enable_testing()

add_subdirectory("${CMAKE_SOURCE_DIR}/tools/ppa_stub"
                 "${CMAKE_BINARY_DIR}/ppa_stub")

add_subdirectory("${CMAKE_SOURCE_DIR}/tools/dll_plugin1"
                 "${CMAKE_BINARY_DIR}/dll_plugin1")

# Convert paths to native Windows format (backslashes) for OpenCppCoverage
file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}" CMAKE_SOURCE_DIR_NATIVE)
file(TO_NATIVE_PATH "${OUTPUT_DIRECTORY}" OUTPUT_DIRECTORY_NATIVE)

# Include tests1 definition
include(${CMAKE_SOURCE_DIR}/src/test/cmake/tests1.cmake)

if(OpenCppCoverage_EXECUTABLE)
  add_test(NAME CoverageTest
    COMMAND ${OpenCppCoverage_EXECUTABLE}
      --export_type cobertura:${CMAKE_SOURCE_DIR_NATIVE}\\tests1-coverage.xml
      --modules ${OUTPUT_DIRECTORY_NATIVE}\\tests1.exe
      --sources ${CMAKE_SOURCE_DIR_NATIVE}
      --excluded_sources ${CMAKE_SOURCE_DIR_NATIVE}\\build
      --working_dir ${OUTPUT_DIRECTORY_NATIVE}
      --cover_children
      --
      $<TARGET_FILE:tests1>
      --gtest_output=xml:${CMAKE_SOURCE_DIR}/tests1-googletest.xml
    CONFIGURATIONS Debug
  )
endif()

add_test(NAME UnitTest
  COMMAND $<TARGET_FILE:tests1>
    --gtest_output=xml:${CMAKE_SOURCE_DIR}/tests1-googletest.xml
  WORKING_DIRECTORY ${OUTPUT_DIRECTORY}
  CONFIGURATIONS Release
)
