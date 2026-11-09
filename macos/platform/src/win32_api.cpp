/*! @file
	@brief Win32 API 的替代实现（非 Windows 平台）

	声明见 macos/platform/include/sakura_platform.h。

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <ctime>
#include <string>
#include <thread>

#include <dlfcn.h>
#include <sys/stat.h>

#if defined(__APPLE__)
#  include <os/log.h>
#endif

namespace {

// 1601-01-01 到 1970-01-01 的 100 纳秒数
constexpr long long kEpochDiff100ns = 116444736000000000LL;

long long FileTimeToUnix100ns(const FILETIME& ft)
{
	const auto v = (static_cast<unsigned long long>(ft.dwHighDateTime) << 32) | ft.dwLowDateTime;
	return static_cast<long long>(v) - kEpochDiff100ns;
}

void TmToSystemTime(const std::tm& tm, int ms, SYSTEMTIME* st)
{
	st->wYear = static_cast<WORD>(tm.tm_year + 1900);
	st->wMonth = static_cast<WORD>(tm.tm_mon + 1);
	st->wDayOfWeek = static_cast<WORD>(tm.tm_wday);
	st->wDay = static_cast<WORD>(tm.tm_mday);
	st->wHour = static_cast<WORD>(tm.tm_hour);
	st->wMinute = static_cast<WORD>(tm.tm_min);
	st->wSecond = static_cast<WORD>(tm.tm_sec);
	st->wMilliseconds = static_cast<WORD>(ms);
}

std::tm SystemTimeToTm(const SYSTEMTIME& st)
{
	std::tm tm{};
	tm.tm_year = st.wYear - 1900;
	tm.tm_mon = st.wMonth - 1;
	tm.tm_mday = st.wDay;
	tm.tm_hour = st.wHour;
	tm.tm_min = st.wMinute;
	tm.tm_sec = st.wSecond;
	return tm;
}

bool IsValidSystemTime(const SYSTEMTIME& st)
{
	return st.wYear >= 1601 && st.wYear <= 30827 && st.wMonth >= 1 && st.wMonth <= 12
		&& st.wDay >= 1 && st.wDay <= 31 && st.wHour < 24 && st.wMinute < 60 && st.wSecond < 60
		&& st.wMilliseconds < 1000;
}

} // namespace

BOOL FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime)
{
	if (!lpFileTime || !lpSystemTime) { errno = EINVAL; return FALSE; }
	const auto v = (static_cast<unsigned long long>(lpFileTime->dwHighDateTime) << 32) | lpFileTime->dwLowDateTime;
	if (v > 0x7FFFFFFFFFFFFFFFULL) { errno = EINVAL; return FALSE; }
	const long long t100 = FileTimeToUnix100ns(*lpFileTime);
	long long secs = t100 / 10000000;
	long long rem = t100 % 10000000;
	if (rem < 0) { rem += 10000000; --secs; }
	const std::time_t t = static_cast<std::time_t>(secs);
	std::tm tm{};
	if (!gmtime_r(&t, &tm)) return FALSE;
	TmToSystemTime(tm, static_cast<int>(rem / 10000), lpSystemTime);
	return TRUE;
}

BOOL SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime)
{
	if (!lpSystemTime || !lpFileTime || !IsValidSystemTime(*lpSystemTime)) { errno = EINVAL; return FALSE; }
	std::tm tm = SystemTimeToTm(*lpSystemTime);
	const std::time_t t = timegm(&tm);
	const auto v = static_cast<unsigned long long>((static_cast<long long>(t) * 10000000) + lpSystemTime->wMilliseconds * 10000LL + kEpochDiff100ns);
	lpFileTime->dwLowDateTime = static_cast<DWORD>(v & 0xFFFFFFFFULL);
	lpFileTime->dwHighDateTime = static_cast<DWORD>(v >> 32);
	return TRUE;
}

BOOL SystemTimeToTzSpecificLocalTime(const void* lpTimeZone, const SYSTEMTIME* lpUniversalTime, LPSYSTEMTIME lpLocalTime)
{
	if (lpTimeZone || !lpUniversalTime || !lpLocalTime || !IsValidSystemTime(*lpUniversalTime)) { errno = EINVAL; return FALSE; }
	std::tm tm = SystemTimeToTm(*lpUniversalTime);
	const std::time_t t = timegm(&tm);
	std::tm local{};
	if (!localtime_r(&t, &local)) return FALSE;
	TmToSystemTime(local, lpUniversalTime->wMilliseconds, lpLocalTime);
	return TRUE;
}

void GetLocalTime(LPSYSTEMTIME lpSystemTime)
{
	const auto now = std::chrono::system_clock::now();
	const std::time_t t = std::chrono::system_clock::to_time_t(now);
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
	std::tm tm{};
	localtime_r(&t, &tm);
	TmToSystemTime(tm, static_cast<int>(ms), lpSystemTime);
}

void GetSystemTime(LPSYSTEMTIME lpSystemTime)
{
	const auto now = std::chrono::system_clock::now();
	const std::time_t t = std::chrono::system_clock::to_time_t(now);
	const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % 1000;
	std::tm tm{};
	gmtime_r(&t, &tm);
	TmToSystemTime(tm, static_cast<int>(ms), lpSystemTime);
}

ULONGLONG GetTickCount64()
{
	return static_cast<ULONGLONG>(std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::steady_clock::now().time_since_epoch()).count());
}

DWORD GetTickCount()
{
	return static_cast<DWORD>(GetTickCount64());
}

void Sleep(DWORD dwMilliseconds)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(dwMilliseconds));
}

int MulDiv(int nNumber, int nNumerator, int nDenominator)
{
	if (nDenominator == 0) return -1;
	const long long num = static_cast<long long>(nNumber) * nNumerator;
	long long q = num / nDenominator;
	const long long r = num % nDenominator;
	// Windows と同じく、余りの 2 倍が除数以上なら絶対値方向に丸める
	if (2 * (r < 0 ? -r : r) >= (nDenominator < 0 ? -static_cast<long long>(nDenominator) : nDenominator)) {
		q += ((num < 0) != (nDenominator < 0)) ? -1 : 1;
	}
	if (q > INT32_MAX || q < INT32_MIN) return -1;
	return static_cast<int>(q);
}

void OutputDebugStringA(const char* lpOutputString)
{
	if (!lpOutputString) return;
#if defined(__APPLE__)
	os_log_debug(OS_LOG_DEFAULT, "%{public}s", lpOutputString);
#endif
	std::fputs(lpOutputString, stderr);
}

void OutputDebugStringW(const WCHAR* lpOutputString)
{
	if (!lpOutputString) return;
	OutputDebugStringA(sakura::u16::to_utf8(lpOutputString).c_str());
}

void DebugBreak()
{
#if defined(__has_builtin)
#  if __has_builtin(__builtin_debugtrap)
	__builtin_debugtrap();
	return;
#  endif
#endif
	std::raise(SIGTRAP);
}

BOOL FreeLibrary(HMODULE hLibModule)
{
	return (hLibModule && ::dlclose(reinterpret_cast<void*>(hLibModule)) == 0) ? TRUE : FALSE;
}

static thread_local DWORD t_lastError = 0;

DWORD GetLastError()
{
	return t_lastError;
}

void SetLastError(DWORD dwErrCode)
{
	t_lastError = dwErrCode;
}

// ---------------------------------------------------------------------------
// 文件
// ---------------------------------------------------------------------------

DWORD GetFileAttributesW(const WCHAR* lpFileName)
{
	if (!lpFileName) { SetLastError(ERROR_INVALID_PARAMETER); return INVALID_FILE_ATTRIBUTES; }
	const std::string path = sakura::u16::to_utf8(lpFileName);
	struct stat st {};
	if (::stat(path.c_str(), &st) != 0) { SetLastError(ERROR_FILE_NOT_FOUND); return INVALID_FILE_ATTRIBUTES; }
	DWORD attr = 0;
	if (S_ISDIR(st.st_mode)) attr |= FILE_ATTRIBUTE_DIRECTORY;
	if ((st.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) attr |= FILE_ATTRIBUTE_READONLY;
	const auto slash = path.find_last_of('/');
	const auto name = slash == std::string::npos ? path : path.substr(slash + 1);
	if (name.size() > 1 && name[0] == '.' && name != "..") attr |= FILE_ATTRIBUTE_HIDDEN;
	return attr ? attr : FILE_ATTRIBUTE_NORMAL;
}

BOOL SetFileAttributesW(const WCHAR* lpFileName, DWORD dwFileAttributes)
{
	if (!lpFileName) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
	const std::string path = sakura::u16::to_utf8(lpFileName);
	struct stat st {};
	if (::stat(path.c_str(), &st) != 0) { SetLastError(ERROR_FILE_NOT_FOUND); return FALSE; }
	mode_t mode = st.st_mode & 07777;
	if (dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
		mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
	} else if ((mode & (S_IWUSR | S_IWGRP | S_IWOTH)) == 0) {
		mode |= S_IWUSR;
	}
	if (mode != (st.st_mode & 07777) && ::chmod(path.c_str(), mode) != 0) { SetLastError(ERROR_ACCESS_DENIED); return FALSE; }
	return TRUE;
}

FILE* _wfopen(const WCHAR* filename, const WCHAR* mode)
{
	if (!filename || !mode) { errno = EINVAL; return nullptr; }
	// 去掉 Windows 专有的模式（ccs=… 和文本模式 t），macOS 的 fopen 不需要
	std::string m;
	for (const WCHAR* p = mode; *p && *p != u','; ++p) {
		if (*p != u't' && *p != u' ') m.push_back(static_cast<char>(*p));
	}
	return std::fopen(sakura::u16::to_utf8(filename).c_str(), m.c_str());
}
