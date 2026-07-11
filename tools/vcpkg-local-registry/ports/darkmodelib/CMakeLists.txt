cmake_minimum_required(VERSION 3.20)
project(darkmodelib LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED DARKMODELIB_ROOT)
  message(FATAL_ERROR "DARKMODELIB_ROOT is required")
endif()

file(GLOB DARKMODELIB_SOURCES
  "${DARKMODELIB_ROOT}/src/*.cpp"
)

add_compile_definitions(
  UNICODE
  _UNICODE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
  NOMINMAX
)

if(MSVC)
  add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    VC_EXTRALEAN
  )

  add_compile_options(
    /W4
    /sdl
    /permissive-
    /utf-8
  )

  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(MINGW)
  add_compile_options(
    -finput-charset=utf-8
    -fexec-charset=utf-8
    -Wdeprecated-declarations
    -Wno-trigraphs
  )
endif()

add_library(darkmodelib STATIC ${DARKMODELIB_SOURCES})
add_library(darkmodelib::darkmodelib ALIAS darkmodelib)

target_compile_features(darkmodelib PUBLIC cxx_std_20)

target_include_directories(darkmodelib
  PRIVATE
    "${DARKMODELIB_ROOT}/include"
  PUBLIC
    "$<INSTALL_INTERFACE:include>"
)

target_compile_definitions(darkmodelib
  PRIVATE
    _WINDOWS
    STRICT_TYPED_ITEMIDS
    SUPPORT_UTF8
    _DARKMODELIB_NO_INI_CONFIG
)

target_link_libraries(darkmodelib
  PUBLIC
    comctl32
    dwmapi
    gdi32
    shlwapi
    uxtheme
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
  "${DARKMODELIB_ROOT}/include/DarkModeSubclass.h"
  DESTINATION include
)

install(TARGETS darkmodelib EXPORT darkmodelib-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT darkmodelib-targets
  NAMESPACE darkmodelib::
  DESTINATION share/darkmodelib
  FILE darkmodelib-targets.cmake
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/darkmodelib-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/darkmodelib-config.cmake"
  INSTALL_DESTINATION share/darkmodelib
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/darkmodelib-config-version.cmake"
  VERSION 0.48.0
  COMPATIBILITY ExactVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/darkmodelib-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/darkmodelib-config-version.cmake"
  DESTINATION share/darkmodelib
)
