#!/bin/bash
# 阶段 1 第 12 步：在非 Windows 环境下用 clang 对原版代码做语法检查，统计编译错误。
#
# 做法：用空的 Windows SDK 头文件（stub）代替真实头文件，让编译器把所有
# Win32 依赖都报成“未声明的标识符”，从而得到完整的依赖清单。
#
# 用法：macos/tools/check_compile.sh <输出目录> <sakura_core 下的目录...>
# 例：  macos/tools/check_compile.sh /tmp/sakura-check basis convert mem io grep parse config docplus debug charset doc types
#
# 环境变量 SAKURA_PLATFORM=1 时（第 13 步起），把 macos/platform/include 放在 include 路径最前面，
# 使用 Mac 版的 StdAfx.h 和 Win32 兼容定义 sakura_platform.h。
set -u
OUT=$1; shift
ROOT=$(cd "$(dirname "$0")/../.." && pwd)
WORK=$OUT/work; STUB=$WORK/stub; GEN=$WORK/gen
mkdir -p "$OUT/logs" "$STUB" "$GEN"

# Funccode_define.h / Funccode_enum.h 由 hsrc 生成
python3 "$ROOT/src/main/py/header_make.py" -in="$ROOT/sakura_core/Funccode_x.hsrc" -out="$GEN/Funccode_define.h" -mode=define >/dev/null
python3 "$ROOT/src/main/py/header_make.py" -in="$ROOT/sakura_core/Funccode_x.hsrc" -out="$GEN/Funccode_enum.h" -mode=enum -enum=EFunctionCode >/dev/null

# Windows SDK / MSVC 专有头文件，以及未检出的子模块头文件，全部用空文件代替
for h in Windows.h windows.h ntstatus.h windowsx.h bcrypt.h comdef.h CommCtrl.h commctrl.h HtmlHelp.h \
         imm.h oaidl.h shellapi.h ShlDisp.h ShlObj.h Uxtheme.h vsstyle.h wrl.h wrl/client.h tchar.h mbstring.h \
         io.h direct.h shlwapi.h Shlwapi.h objidl.h ole2.h vadefs.h WinSpool.h CommDlg.h OleAuto.h ObjIdl.h \
         Unknwn.h wincodec.h WTypes.h Commctrl.h ActivScp.h cmigemo/migemo.h; do
  mkdir -p "$STUB/$(dirname "$h")"; : > "$STUB/$h"
done

PLATFORM_INC=()
if [ "${SAKURA_PLATFORM:-0}" = 1 ]; then
  PLATFORM_INC=(-I "$ROOT/macos/platform/include")
fi

CXX=${CXX:-clang++}
cd "$ROOT"
for d in "$@"; do
  for f in $(find "sakura_core/$d" -name '*.cpp' | sort); do
    log="$OUT/logs/$(echo "$f" | tr / _).log"
    "$CXX" -std=c++20 -fsyntax-only -ferror-limit=0 -w -DNOMINMAX \
      "${PLATFORM_INC[@]}" -isystem "$STUB" -I sakura_core -I src/main/cpp -I src/main/resources -I "$GEN" \
      "$f" > "$log" 2>&1
    echo "$f $(grep -c ' error: ' "$log")"
  done
done
