cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME sakura)
project(${PROJECT_NAME} LANGUAGES CXX)

# Include sakura.cmake for funccode definitions
include(${CMAKE_SOURCE_DIR}/src/main/cmake/sakura.cmake)

# Create language DLL projects
# en-US: 1033 (0x0409) - English (United States)
# zh-CN: 2052 (0x0804) - Chinese (Simplified, PRC)
create_language_dll(en-US 1033)
create_language_dll(zh-CN 2052)
