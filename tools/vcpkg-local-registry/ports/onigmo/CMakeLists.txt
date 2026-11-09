cmake_minimum_required(VERSION 3.20)
project(onigmo LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

if(NOT DEFINED ONIGMO_ROOT)
  message(FATAL_ERROR "ONIGMO_ROOT is required")
endif()

set(ONIGMO_SOURCES
  "${ONIGMO_ROOT}/regcomp.c"
  "${ONIGMO_ROOT}/regenc.c"
  "${ONIGMO_ROOT}/regerror.c"
  "${ONIGMO_ROOT}/regexec.c"
  "${ONIGMO_ROOT}/regext.c"
  "${ONIGMO_ROOT}/reggnu.c"
  "${ONIGMO_ROOT}/regparse.c"
  "${ONIGMO_ROOT}/regposerr.c"
  "${ONIGMO_ROOT}/regposix.c"
  "${ONIGMO_ROOT}/regsyntax.c"
  "${ONIGMO_ROOT}/regtrav.c"
  "${ONIGMO_ROOT}/regversion.c"
  "${ONIGMO_ROOT}/st.c"
  "${ONIGMO_ROOT}/enc/ascii.c"
  "${ONIGMO_ROOT}/enc/big5.c"
  "${ONIGMO_ROOT}/enc/euc_jp.c"
  "${ONIGMO_ROOT}/enc/euc_kr.c"
  "${ONIGMO_ROOT}/enc/euc_tw.c"
  "${ONIGMO_ROOT}/enc/gb18030.c"
  "${ONIGMO_ROOT}/enc/iso_8859_1.c"
  "${ONIGMO_ROOT}/enc/iso_8859_2.c"
  "${ONIGMO_ROOT}/enc/iso_8859_3.c"
  "${ONIGMO_ROOT}/enc/iso_8859_4.c"
  "${ONIGMO_ROOT}/enc/iso_8859_5.c"
  "${ONIGMO_ROOT}/enc/iso_8859_6.c"
  "${ONIGMO_ROOT}/enc/iso_8859_7.c"
  "${ONIGMO_ROOT}/enc/iso_8859_8.c"
  "${ONIGMO_ROOT}/enc/iso_8859_9.c"
  "${ONIGMO_ROOT}/enc/iso_8859_10.c"
  "${ONIGMO_ROOT}/enc/iso_8859_11.c"
  "${ONIGMO_ROOT}/enc/iso_8859_13.c"
  "${ONIGMO_ROOT}/enc/iso_8859_14.c"
  "${ONIGMO_ROOT}/enc/iso_8859_15.c"
  "${ONIGMO_ROOT}/enc/iso_8859_16.c"
  "${ONIGMO_ROOT}/enc/koi8_r.c"
  "${ONIGMO_ROOT}/enc/koi8_u.c"
  "${ONIGMO_ROOT}/enc/shift_jis.c"
  "${ONIGMO_ROOT}/enc/unicode.c"
  "${ONIGMO_ROOT}/enc/utf_8.c"
  "${ONIGMO_ROOT}/enc/utf_16be.c"
  "${ONIGMO_ROOT}/enc/utf_16le.c"
  "${ONIGMO_ROOT}/enc/utf_32be.c"
  "${ONIGMO_ROOT}/enc/utf_32le.c"
  "${ONIGMO_ROOT}/enc/windows_1250.c"
  "${ONIGMO_ROOT}/enc/windows_1251.c"
  "${ONIGMO_ROOT}/enc/windows_1252.c"
  "${ONIGMO_ROOT}/enc/windows_1253.c"
  "${ONIGMO_ROOT}/enc/windows_1254.c"
  "${ONIGMO_ROOT}/enc/windows_1257.c"
  "${ONIGMO_ROOT}/enc/windows_31j.c"
)

add_library(onigmo STATIC ${ONIGMO_SOURCES})
add_library(onigmo::onigmo ALIAS onigmo)

target_include_directories(onigmo
  PUBLIC
    "$<BUILD_INTERFACE:${ONIGMO_ROOT}>"
    "$<BUILD_INTERFACE:${ONIGMO_ROOT}/enc/unicode>"
    "$<INSTALL_INTERFACE:include>"
)

target_compile_definitions(onigmo
  PRIVATE
    HAVE_CONFIG_H
    ONIG_EXTERN=extern
)

if(MSVC)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
  target_compile_options(onigmo PRIVATE /utf-8)
endif()

install(FILES
  "${ONIGMO_ROOT}/onigmo.h"
  "${ONIGMO_ROOT}/onigmognu.h"
  "${ONIGMO_ROOT}/onigmoposix.h"
  DESTINATION include
)

install(TARGETS onigmo EXPORT onigmo-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

install(EXPORT onigmo-targets
  NAMESPACE onigmo::
  DESTINATION share/onigmo
  FILE onigmo-targets.cmake
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/onigmo-config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/onigmo-config.cmake"
  INSTALL_DESTINATION share/onigmo
)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/onigmo-config-version.cmake"
  VERSION 6.2.0
  COMPATIBILITY ExactVersion
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/onigmo-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/onigmo-config-version.cmake"
  DESTINATION share/onigmo
)
