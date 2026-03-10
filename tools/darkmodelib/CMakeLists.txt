cmake_minimum_required(VERSION 3.20)
project(darkmodelib LANGUAGES CXX)

include(GNUInstallDirs)

set(DARKMODELIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../externals/darkmodelib")

file(GLOB DARKMODELIB_SOURCES
  ${DARKMODELIB_ROOT}/src/*.cpp
)

# add global definitions
add_compile_definitions(
  UNICODE
  _UNICODE
  $<$<CONFIG:Debug>:_DEBUG>
  $<$<CONFIG:Release>:NDEBUG>
  NOMINMAX
)

if(MSVC)
  # add MSVC definitions
  add_compile_definitions(
    WIN32_LEAN_AND_MEAN
    VC_EXTRALEAN
  )

  # add MSVC compiler options
  add_compile_options(
    /W4
    /sdl
    /permissive-
    /utf-8
  )

  # VCランタイムを指定する（/MTd, /MTにする）
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif(MSVC)

if(MINGW)
  add_compile_options(
    -finput-charset=utf-8
    -fexec-charset=utf-8
    -Wdeprecated-declarations
    -Wno-trigraphs
  )
endif(MINGW)

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

# add link dependencies for darkmodelib
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

# Set darkmode.lib
set_target_properties(darkmodelib
  PROPERTIES
    OUTPUT_NAME darkmode
)

install(FILES
  ${DARKMODELIB_ROOT}/include/DarkModeSubclass.h
  DESTINATION include
)

install(TARGETS darkmodelib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

set(DARKMODELIB_PC_FILE "${CMAKE_CURRENT_BINARY_DIR}/darkmodelib.pc")
file(READ
  "${CMAKE_CURRENT_LIST_DIR}/darkmodelib.pc.in"
  DARKMODELIB_PC_TEMPLATE_CONTENT
)
string(CONFIGURE
  "${DARKMODELIB_PC_TEMPLATE_CONTENT}"
  DARKMODELIB_PC_CONTENT
  @ONLY
)
file(GENERATE
  OUTPUT "${DARKMODELIB_PC_FILE}"
  CONTENT "${DARKMODELIB_PC_CONTENT}"
)
install(FILES
  "${DARKMODELIB_PC_FILE}"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig
)
