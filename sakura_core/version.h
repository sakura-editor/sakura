#pragma once

#include "config/build_config.h"
#include "githash.h"

// バージョン定義 //
// ver a.b.c.d
// 例: ver 2.3.2.0
//       a  => 2
//       b  => 3
//       c  => 2
//       d  => 0
#define VER_A   2 // a of ver a.b.c.d
#define VER_B   3 // b of ver a.b.c.d
#define VER_C   2 // c of ver a.b.c.d
#define VER_D   0 // d of ver a.b.c.d

#define TO_STR(arg)                            #arg
#define MAKE_VERSION_STR(a, b, c, d, sep)      TO_STR(a) sep TO_STR(b) sep TO_STR(c) sep TO_STR(d)
#define MAKE_VERSION_STR_PERIOD(a, b, c, d)    MAKE_VERSION_STR(a, b, c, d, ".")
#define MAKE_VERSION_COMMA(a, b, c, d)         a, b, c, d

#define PR_VER_STR  MAKE_VERSION_STR_PERIOD(VER_A, VER_B, VER_C, VER_D)
#define PR_VER      MAKE_VERSION_COMMA(VER_A, VER_B, VER_C, VER_D)

#ifdef _UNICODE
#define VER_CHARSET "UNICODE"
#else
#define VER_CHARSET "ANSI"
#endif

#ifdef _WIN64
#define VER_PLATFORM "64bit"
#else
#define VER_PLATFORM "32bit"
#endif

#ifdef _DEBUG
#define VER_CONFIG "DEBUG"
#else
#define VER_CONFIG ""
#endif

#ifdef _DEBUG
#define SPACE_WHEN_DEBUG " "
#else
#define SPACE_WHEN_DEBUG ""
#endif

#if defined(ALPHA_VERSION)
#pragma message("----------------------------------------------------------------------------------------")
#pragma message("---  This is an alpha version and under development. Be careful to use this version. ---")
#pragma message("----------------------------------------------------------------------------------------")
#endif

#ifdef ALPHA_VERSION
#define ALPHA_VERSION_STR            "Alpha Version"
#define ALPHA_VERSION_STR_WITH_SPACE " " ALPHA_VERSION_STR
#else
#define ALPHA_VERSION_STR_WITH_SPACE ""
#endif

// バージョン情報埋め込み用 Git ハッシュ文字列 (存在しない場合には空文字列)
#ifdef GIT_SHORT_COMMIT_HASH
#define VER_GIT_SHORTHASH " (" GIT_SHORT_COMMIT_HASH ")"
#else
#define VER_GIT_SHORTHASH ""
#endif

// リソース埋め込み用バージョン文字列 //
// e.g. "2.3.2.0 (4a0de579) UNICODE 64bit DEBUG" … デバッグビルド時の例 //
// e.g. "2.3.2.0 (4a0de579) UNICODE 64bit"       … リリースビルド時の例 //
// e.g. "2.3.2.0 UNICODE 64bit"                  … Git 情報無い場合の例 //
#define RESOURCE_VERSION_STRING(_VersionString) _VersionString VER_GIT_SHORTHASH " " VER_CHARSET " " VER_PLATFORM SPACE_WHEN_DEBUG VER_CONFIG ALPHA_VERSION_STR_WITH_SPACE
