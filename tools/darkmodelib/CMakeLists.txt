cmake_minimum_required(VERSION 3.20)
project(darkmodelib LANGUAGES CXX)

set(DARKMODELIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../externals/darkmodelib")

file(GLOB DARKMODELIB_SOURCES
  ${DARKMODELIB_ROOT}/src/*.cpp
)

if(MSVC)
  # add global definitions
  add_compile_definitions(
    UNICODE
    _UNICODE
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    VC_EXTRALEAN
  )

  # add global compiler options
  add_compile_options(
    /W4
    /sdl
    /permissive-
    /utf-8
  )

  # VCランタイムを指定する（/MTd, /MTにする）
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

# Create darkmodelib static library
add_library(darkmodelib STATIC ${DARKMODELIB_SOURCES})

# Set C++ standard for darkmodelib
target_compile_features(darkmodelib PUBLIC cxx_std_20)

# Add include directories for darkmodelib
target_include_directories(darkmodelib
  PRIVATE
    ${DARKMODELIB_ROOT}/include
  SYSTEM
  INTERFACE
    $<INSTALL_INTERFACE:include>
)

# add definitions for darkmodelib
target_compile_definitions(darkmodelib
  PRIVATE
    _WINDOWS
    STRICT_TYPED_ITEMIDS
    SUPPORT_UTF8
    _DARKMODELIB_NO_INI_CONFIG
)

if(MSVC)
  target_compile_options(darkmodelib
    PRIVATE
      /Zc:__cplusplus
      /wd26490
      $<$<CONFIG:Release>:/MP>
      $<$<CONFIG:Release>:/Gw>
  )
endif()

set_target_properties(darkmodelib
  PROPERTIES
    OUTPUT_NAME darkmode
)

install(FILES
  ${DARKMODELIB_ROOT}/include/DarkModeSubclass.h
  DESTINATION include
)

install(TARGETS darkmodelib
  ARCHIVE DESTINATION lib/$<CONFIG>
  LIBRARY DESTINATION lib/$<CONFIG>
  RUNTIME DESTINATION bin/$<CONFIG>
)
