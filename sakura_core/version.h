#pragma once

#include "githash.h"

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

#if _WIN64
#define ALPHA_VERSION
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

// リソース埋め込み用バージョン文字列
// e.g. "2.3.2.0 (4a0de579) UNICODE 64bit DEBUG" … デバッグビルド時の例
// e.g. "2.3.2.0 (4a0de579) UNICODE 64bit"       … リリースビルド時の例
// e.g. "2.3.2.0 UNICODE 64bit"                  … Git 情報無い場合の例
#define RESOURCE_VERSION_STRING(_VersionString) _VersionString VER_GIT_SHORTHASH " " VER_CHARSET " " VER_PLATFORM SPACE_WHEN_DEBUG VER_CONFIG ALPHA_VERSION_STR_WITH_SPACE
