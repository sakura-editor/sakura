# Sakura Editor for Mac：核心库（libsakura_core）和单元测试
#
# 阶段 1 第 16 步起使用。macOS 和 Linux 都能构建（Linux 只用于开发时快速检查）。
#
#   cmake -S macos/core -B build/mac-core -DCMAKE_BUILD_TYPE=Debug
#   cmake --build build/mac-core -j
#   ctest --test-dir build/mac-core --output-on-failure
#
# 核心库只包含已经移植到非 Windows 平台的目录，见 SAKURA_CORE_DIRS。
# 后续步骤每移植一个目录，就加到这里。
cmake_minimum_required(VERSION 3.24)
project(sakura_core_mac LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_filename_component(SAKURA_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(SAKURA_CORE "${SAKURA_ROOT}/sakura_core")
set(GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
set(STUB_DIR "${CMAKE_CURRENT_BINARY_DIR}/winsdk_stub")

# ---------------------------------------------------------------------------
# Windows SDK 头文件的空替身。原版有些头文件直接 #include <Windows.h>；
# 需要的定义已经由 macos/platform/include/StdAfx.h 提供。
# 大小写不同的写法都要有（Linux 区分大小写），所以在配置时生成，而不是放进仓库。
# ---------------------------------------------------------------------------
foreach(h Windows.h windows.h windowsx.h ntstatus.h tchar.h mbstring.h io.h process.h
          Shlwapi.h shlwapi.h shellapi.h ShlObj.h ObjIdl.h objidl.h CommCtrl.h Commctrl.h
          CommDlg.h commdlg.h Uxtheme.h uxtheme.h vsstyle.h vssym32.h HtmlHelp.h imm.h
          oaidl.h ole2.h OleAuto.h ObjBase.h Unknwn.h WTypes.h comdef.h wrl.h wrl/client.h
          bcrypt.h ShlDisp.h shobjidl.h wincodec.h WinSpool.h dwmapi.h crtdbg.h InitGuid.h
          intrin.h vadefs.h DbgHelp.h ActivScp.h)
  if(NOT EXISTS "${STUB_DIR}/${h}")
    file(WRITE "${STUB_DIR}/${h}" "// Windows SDK stub for non-Windows builds\n#pragma once\n")
  endif()
endforeach()

# ---------------------------------------------------------------------------
# Funccode_define.h / Funccode_enum.h（与 Windows 版相同，由 hsrc 生成）
# ---------------------------------------------------------------------------
find_package(Python3 REQUIRED COMPONENTS Interpreter)
set(HSRC "${SAKURA_CORE}/Funccode_x.hsrc")
set(HEADER_MAKE "${SAKURA_ROOT}/src/main/py/header_make.py")
add_custom_command(
  OUTPUT "${GEN_DIR}/Funccode_define.h" "${GEN_DIR}/Funccode_enum.h"
  COMMAND ${CMAKE_COMMAND} -E make_directory "${GEN_DIR}"
  COMMAND Python3::Interpreter "${HEADER_MAKE}" "-in=${HSRC}" "-out=${GEN_DIR}/Funccode_define.h" -mode=define
  COMMAND Python3::Interpreter "${HEADER_MAKE}" "-in=${HSRC}" "-out=${GEN_DIR}/Funccode_enum.h" -mode=enum -enum=EFunctionCode
  DEPENDS "${HSRC}" "${HEADER_MAKE}"
  COMMENT "Generating Funccode headers"
  VERBATIM)
add_custom_target(sakura_funccode DEPENDS "${GEN_DIR}/Funccode_define.h" "${GEN_DIR}/Funccode_enum.h")

# ---------------------------------------------------------------------------
# 核心库
# ---------------------------------------------------------------------------
set(SAKURA_CORE_DIRS basis config convert debug docplus io mem parse)
# 暂不编入核心库的文件（依赖尚未移植的模块，或 Mac 版不需要）
set(SAKURA_CORE_EXCLUDE
  # Mac 版不需要
  basis/CErrorInfo.cpp            # COM 的 IErrorInfo 实现
  io/CZipFile.cpp                 # 用 Windows Shell 解压 zip，只给插件安装用；Mac 版不支持 DLL 插件
  # 第 17 步（charset）移植后加入
  io/CTextStream.cpp              # 依赖 util/tchar_convert（代码页转换）
  # 第 19 步（doc、types）移植后加入
  io/CFile.cpp                    # 依赖 window/CEditWnd.h
  io/CFileLoad.cpp                # 依赖 charset/CESI 和 window/CEditWnd.h
  docplus/CDiffManager.cpp        # 依赖 window/CEditWnd.h 和绘制
  docplus/CModifyManager.cpp      # 依赖 doc/CEditDoc 和 COM 的 VARIANT（宏接口）
)
# grep/GrepMessageFormat.cpp 依赖 env/DLLSHAREDATA，第 20 步（env）移植后把 grep 加进 SAKURA_CORE_DIRS

set(SAKURA_CORE_SOURCES)
foreach(d ${SAKURA_CORE_DIRS})
  file(GLOB files CONFIGURE_DEPENDS "${SAKURA_CORE}/${d}/*.cpp")
  list(APPEND SAKURA_CORE_SOURCES ${files})
endforeach()
foreach(x ${SAKURA_CORE_EXCLUDE})
  list(REMOVE_ITEM SAKURA_CORE_SOURCES "${SAKURA_CORE}/${x}")
endforeach()
# 核心目录之外、已经能在 Mac 上编译的单个文件（核心目录依赖它们）
set(SAKURA_CORE_EXTRA_SOURCES
  sakura_core/charset/charcode.cpp   # 文字種・文字幅（Mac 版は Core Text の測定関数を使う）
  sakura_core/util/format.cpp
  sakura_core/util/string_ex.cpp
  sakura_core/util/string_ex2.cpp
)
list(TRANSFORM SAKURA_CORE_EXTRA_SOURCES PREPEND "${SAKURA_ROOT}/")
list(APPEND SAKURA_CORE_SOURCES ${SAKURA_CORE_EXTRA_SOURCES})

add_library(sakura_core STATIC
  ${SAKURA_CORE_SOURCES}
  "${SAKURA_ROOT}/macos/platform/src/win32_api.cpp"
  "${SAKURA_ROOT}/macos/platform/src/ui_bridge.cpp")
add_dependencies(sakura_core sakura_funccode)
target_include_directories(sakura_core PUBLIC
  "${SAKURA_ROOT}/macos/platform/include"   # Mac 版 StdAfx.h 优先
  "${SAKURA_CORE}"
  "${SAKURA_ROOT}/src/main/cpp"
  "${SAKURA_ROOT}/src/main/resources"
  "${GEN_DIR}")
target_include_directories(sakura_core SYSTEM PUBLIC "${STUB_DIR}")
target_compile_definitions(sakura_core PUBLIC NOMINMAX $<$<CONFIG:Debug>:_DEBUG>)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  # 原版代码在 MSVC 下编译，先关掉与移植无关的大量警告，只保留错误
  target_compile_options(sakura_core PRIVATE -Wno-everything)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(sakura_core PRIVATE -w)
  endif()
endif()

# ---------------------------------------------------------------------------
# 单元测试（GoogleTest）
# ---------------------------------------------------------------------------
option(SAKURA_BUILD_TESTS "Build unit tests" ON)
if(SAKURA_BUILD_TESTS)
  include(FetchContent)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/releases/download/v1.17.0/googletest-1.17.0.tar.gz
    URL_HASH SHA256=65fab701d9829d38cb77c14acdc431d2108bfdbf8979e40eb8ae567edf10b27c
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  include("${CMAKE_CURRENT_SOURCE_DIR}/tests/tests.cmake")
  add_executable(sakura_core_tests ${SAKURA_CORE_TEST_SOURCES})
  target_include_directories(sakura_core_tests PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/tests"          # Mac 版 pch.h 优先
    "${SAKURA_ROOT}/src/test/resources")
  target_link_libraries(sakura_core_tests PRIVATE sakura_core GTest::gmock_main)
  if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(sakura_core_tests PRIVATE -w)
  endif()
  enable_testing()
  include(GoogleTest)
  gtest_discover_tests(sakura_core_tests DISCOVERY_MODE PRE_TEST)
endif()
