/*! @file
	@brief Mac 版用的预编译头（平台抽象层，阶段 1 第 13 步）

	在 Mac 版的编译中，这个目录排在 include 路径的最前面，
	因此 `#include "StdAfx.h"` 会用这个文件代替 sakura_core/StdAfx.h。

	与原版的区别：
	- 不包含 Windows.h 等 Windows SDK 头文件，改为包含 sakura_platform.h。
	- 不包含 env/DLLSHAREDATA.h。原版靠预编译头“顺带”引入整个共享数据结构，
	  Mac 版要求各文件自己包含真正需要的头文件。

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#pragma once

// 標準C++ヘッダー（追加するときは昇順で。）
#include <algorithm>
#include <array>
#include <atomic>
#include <cassert>
#include <chrono>
#include <climits>
#include <clocale>
#include <cmath>
#include <concepts>
#include <condition_variable>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cwchar>
#include <cwctype>
#include <deque>
#include <exception>
#include <filesystem>
#include <format>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <numeric>
#include <optional>
#include <ranges>
#include <regex>
#include <set>
#include <source_location>
#include <span>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <wchar.h>

// Win32 兼容定义（非 Windows 平台）
#include "sakura_platform.h"

// UTF-16 字符串函数（wcslen 等的 char16_t 版本）
#include "util/wchar16.h"

#if defined(__GNUC__) || defined(__clang__)
#  define NORETURN __attribute__((noreturn))
#else
#  define NORETURN [[noreturn]]
#endif
