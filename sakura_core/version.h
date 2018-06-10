#pragma once

#include "gitrev.h"

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

// リソース埋め込み用バージョン文字列
// e.g. "2.3.2.0 (4a0de579) UNICODE 64bit DEBUG"
#define RESOURCE_VERSION_STRING(VERSION_STRING) VERSION_STRING " (" GIT_SHORT_COMMIT_HASH ") " VER_CHARSET " " VER_PLATFORM " " VER_CONFIG
