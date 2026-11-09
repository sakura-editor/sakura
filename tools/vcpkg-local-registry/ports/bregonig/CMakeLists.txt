cmake_minimum_required(VERSION 3.20)
project(bregonig LANGUAGES CXX RC)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED BREGONIG_ROOT)
  message(FATAL_ERROR "BREGONIG_ROOT is required")
endif()

find_package(onigmo CONFIG REQUIRED)

set(BREGONIG_SOURCES
  "${BREGONIG_ROOT}/src/bregonig.cpp"
  "${BREGONIG_ROOT}/src/subst.cpp"
  "${BREGONIG_ROOT}/src/bsplit.cpp"
  "${BREGONIG_ROOT}/src/btrans.cpp"
  "${BREGONIG_ROOT}/src/sv.cpp"
  "${BREGONIG_ROOT}/src/bregonig.rc"
)

add_library(bregonig SHARED ${BREGONIG_SOURCES})
add_library(bregonig::bregonig ALIAS bregonig)

target_include_directories(bregonig
  PRIVATE
    "${BREGONIG_ROOT}/src"
)

target_compile_definitions(bregonig
  PRIVATE
    _CRT_SECURE_NO_DEPRECATE
    WIN32_LEAN_AND_MEAN
    _BREGEXP_
    USE_ONIGMO_6
    UNICODE
    _UNICODE
)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(bregonig
    PRIVATE
      /utf-8
      /EHsc
      /W3
      /wd4996
  )
endif()

target_link_libraries(bregonig
  PRIVATE
    onigmo::onigmo
)

set_target_properties(bregonig PROPERTIES
  OUTPUT_NAME bregonig
)

install(FILES
  "${BREGONIG_ROOT}/src/bregexp.h"
  DESTINATION include/bregonig
)

install(TARGETS bregonig EXPORT bregonig-targets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT bregonig-targets
  NAMESPACE bregonig::
  DESTINATION share/bregonig
  FILE bregonig-targets.cmake
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/bregonig-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/bregonig-config.cmake"
  INSTALL_DESTINATION share/bregonig
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/bregonig-config-version.cmake"
  VERSION 4.20.0
  COMPATIBILITY ExactVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/bregonig-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/bregonig-config-version.cmake"
  DESTINATION share/bregonig
)
