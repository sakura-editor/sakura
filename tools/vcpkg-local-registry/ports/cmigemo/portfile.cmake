get_filename_component(REPO_ROOT "${CURRENT_PORT_DIR}/../../../.." ABSOLUTE)
set(CMIGEMO_SOURCE_DIR "${REPO_ROOT}/externals/cmigemo")
set(CMIGEMO_DICT_SOURCE_DIR "${REPO_ROOT}/externals/cmigemo-dict")

if(NOT VCPKG_TARGET_IS_WINDOWS)
  message(FATAL_ERROR "cmigemo port supports Windows targets only")
endif()

if(NOT EXISTS "${CMIGEMO_SOURCE_DIR}/src/migemo.h")
  message(FATAL_ERROR "cmigemo source not found: ${CMIGEMO_SOURCE_DIR}")
endif()

if(NOT EXISTS "${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-utf-8.zip")
  message(FATAL_ERROR "cmigemo dictionary zip not found: ${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-utf-8.zip")
endif()

if(NOT EXISTS "${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-cp932.zip")
  message(FATAL_ERROR "cmigemo dictionary zip not found: ${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-cp932.zip")
endif()

if(VCPKG_TARGET_ARCHITECTURE STREQUAL "x64")
  set(TARGET_CPU "x64")
elseif(VCPKG_TARGET_ARCHITECTURE STREQUAL "x86")
  set(TARGET_CPU "x86")
else()
  message(FATAL_ERROR "Unsupported architecture: ${VCPKG_TARGET_ARCHITECTURE}")
endif()

execute_process(
  COMMAND cmd.exe /c "echo %ProgramFiles(x86)%"
  OUTPUT_VARIABLE _PROG_FILES_X86
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
find_program(VSWHERE vswhere
  HINTS
    "${_PROG_FILES_X86}/Microsoft Visual Studio/Installer"
    "$ENV{ProgramFiles}/Microsoft Visual Studio/Installer"
)
if(NOT VSWHERE)
  message(FATAL_ERROR "vswhere not found. Install Visual Studio 2017 or later.")
endif()

execute_process(
  COMMAND "${VSWHERE}" -latest -find "Common7/Tools/VsDevCmd.bat"
  OUTPUT_VARIABLE VSDEVCMD_PATH
  OUTPUT_STRIP_TRAILING_WHITESPACE
  RESULT_VARIABLE VSWHERE_RESULT
)
if(NOT VSWHERE_RESULT EQUAL 0 OR NOT EXISTS "${VSDEVCMD_PATH}")
  message(FATAL_ERROR "VsDevCmd.bat not found.")
endif()
string(REPLACE "/" "\\" VSDEVCMD_PATH "${VSDEVCMD_PATH}")

set(VCPKG_POLICY_DLLS_WITHOUT_LIBS enabled)
set(VCPKG_POLICY_DLLS_IN_STATIC_LIBRARY enabled)

set(BUILD_DIR_REL "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-rel")
set(BUILD_DIR_DBG "${CURRENT_BUILDTREES_DIR}/${TARGET_TRIPLET}-dbg")
file(REMOVE_RECURSE "${BUILD_DIR_REL}" "${BUILD_DIR_DBG}")
file(COPY "${CMIGEMO_SOURCE_DIR}/" DESTINATION "${BUILD_DIR_REL}")
file(COPY "${CMIGEMO_SOURCE_DIR}/" DESTINATION "${BUILD_DIR_DBG}")

file(WRITE "${BUILD_DIR_REL}/build_rel.cmd"
  "@echo off\n"
  "call \"${VSDEVCMD_PATH}\" -host_arch=x64 -arch=${TARGET_CPU}\n"
  "if errorlevel 1 exit /b 1\n"
  "nmake -f compile/Make_mvc.mak DEFINES=-D_CRT_SECURE_NO_WARNINGS\n"
)
vcpkg_execute_required_process(
  COMMAND cmd.exe /c "${BUILD_DIR_REL}/build_rel.cmd"
  WORKING_DIRECTORY "${BUILD_DIR_REL}"
  LOGNAME "build-${TARGET_TRIPLET}-rel"
)

file(WRITE "${BUILD_DIR_DBG}/build_dbg.cmd"
  "@echo off\n"
  "call \"${VSDEVCMD_PATH}\" -host_arch=x64 -arch=${TARGET_CPU}\n"
  "if errorlevel 1 exit /b 1\n"
  "nmake -f compile/Make_mvc.mak DEFINES=-D_CRT_SECURE_NO_WARNINGS DEBUG=1\n"
)
vcpkg_execute_required_process(
  COMMAND cmd.exe /c "${BUILD_DIR_DBG}/build_dbg.cmd"
  WORKING_DIRECTORY "${BUILD_DIR_DBG}"
  LOGNAME "build-${TARGET_TRIPLET}-dbg"
)

set(REL_DLL "${BUILD_DIR_REL}/build/migemo.dll")
set(REL_LIB "${BUILD_DIR_REL}/build/migemo.lib")
set(DBG_DLL "${BUILD_DIR_DBG}/build/migemo.dll")
set(DBG_LIB "${BUILD_DIR_DBG}/build/migemo.lib")
foreach(_required IN ITEMS "${REL_DLL}" "${REL_LIB}" "${DBG_DLL}" "${DBG_LIB}")
  if(NOT EXISTS "${_required}")
    message(FATAL_ERROR "Expected cmigemo artifact not found: ${_required}")
  endif()
endforeach()

file(MAKE_DIRECTORY
  "${CURRENT_PACKAGES_DIR}/bin"
  "${CURRENT_PACKAGES_DIR}/debug/bin"
  "${CURRENT_PACKAGES_DIR}/lib"
  "${CURRENT_PACKAGES_DIR}/debug/lib"
  "${CURRENT_PACKAGES_DIR}/include/cmigemo"
  "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)

file(INSTALL "${REL_DLL}" DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
file(INSTALL "${DBG_DLL}" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")
file(INSTALL "${REL_LIB}" DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
file(INSTALL "${DBG_LIB}" DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

find_program(PWSH NAMES pwsh powershell)
if(NOT PWSH)
  message(FATAL_ERROR "PowerShell not found. Required to convert migemo.h to UTF-8.")
endif()

execute_process(
  COMMAND "${PWSH}" -NoProfile -NonInteractive -ExecutionPolicy Bypass -Command
    "$sjis = [System.Text.Encoding]::GetEncoding(932); $utf8 = [System.Text.UTF8Encoding]::new($true); [System.IO.File]::WriteAllText('${CURRENT_PACKAGES_DIR}/include/cmigemo/migemo.h', [System.IO.File]::ReadAllText('${CMIGEMO_SOURCE_DIR}/src/migemo.h', $sjis), $utf8)"
  RESULT_VARIABLE CMIGEMO_HEADER_CONVERT_RESULT
)
if(NOT CMIGEMO_HEADER_CONVERT_RESULT EQUAL 0)
  message(FATAL_ERROR "Failed to convert migemo.h to UTF-8")
endif()

file(INSTALL
  "${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-utf-8.zip"
  "${CMIGEMO_DICT_SOURCE_DIR}/cmigemo-dict-cp932.zip"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
)

file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/cmigemoConfig.cmake" [[
if(NOT TARGET cmigemo::cmigemo)
  get_filename_component(_CMIGEMO_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
  add_library(cmigemo::cmigemo SHARED IMPORTED)
  set_target_properties(cmigemo::cmigemo PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES   "${_CMIGEMO_ROOT}/include"
    IMPORTED_CONFIGURATIONS         "RELEASE;DEBUG"
    IMPORTED_LOCATION_RELEASE       "${_CMIGEMO_ROOT}/bin/migemo.dll"
    IMPORTED_IMPLIB_RELEASE         "${_CMIGEMO_ROOT}/lib/migemo.lib"
    IMPORTED_LOCATION_DEBUG         "${_CMIGEMO_ROOT}/debug/bin/migemo.dll"
    IMPORTED_IMPLIB_DEBUG           "${_CMIGEMO_ROOT}/debug/lib/migemo.lib"
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO RELEASE
    MAP_IMPORTED_CONFIG_MINSIZEREL     RELEASE
  )
endif()

get_filename_component(cmigemo_PACKAGE_ROOT "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(cmigemo_DICT_UTF8_ZIP  "${cmigemo_PACKAGE_ROOT}/share/cmigemo/cmigemo-dict-utf-8.zip")
set(cmigemo_DICT_CP932_ZIP "${cmigemo_PACKAGE_ROOT}/share/cmigemo/cmigemo-dict-cp932.zip")
]])

file(WRITE "${CURRENT_PACKAGES_DIR}/share/${PORT}/cmigemoConfigVersion.cmake" [[
set(PACKAGE_VERSION "1.3e")
if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
  set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
  set(PACKAGE_VERSION_COMPATIBLE TRUE)
  if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
    set(PACKAGE_VERSION_EXACT TRUE)
  endif()
endif()
]])

configure_file("${CMAKE_CURRENT_LIST_DIR}/usage" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)

file(INSTALL
  "${CMIGEMO_SOURCE_DIR}/doc/LICENSE_MIT.txt"
  DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}"
  RENAME copyright
)