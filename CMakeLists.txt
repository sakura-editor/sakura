cmake_minimum_required(VERSION 2.6.4)

project (SakuraEditor)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(MakefileMake)
add_subdirectory(HeaderMake)
add_subdirectory(sakura_core)
add_subdirectory(sakura_lang_en_US)
