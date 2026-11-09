/*! @file
	@brief util/wchar16.h 的示例与测试（Mac 版移植 阶段 1 第 14 步）

	同一份代码在 Windows（MSVC，WCHAR = wchar_t）和 macOS / Linux（clang，WCHAR = char16_t）上编译运行。
	Windows 上还会把 sakura::u16 的结果和 MSVC 运行库的结果逐一对比，确认语义一致。

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#if defined(_WIN32)
#  include <Windows.h>
#else
#  include "StdAfx.h"   // macos/platform/include/StdAfx.h
#endif
#include "util/std_macro.h"
#include "util/wchar16.h"

#include <climits>
#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace u16 = sakura::u16;

static int g_failed = 0;
static int g_passed = 0;
#define CHECK(cond) do { if (cond) { ++g_passed; } else { ++g_failed; std::printf("FAILED %s:%d: %s\n", __FILE__, __LINE__, #cond); } } while (0)
static bool eq(const WCHAR* a, const WCHAR* b) { return u16::strcmp(a, b) == 0; }
static int call_vsnprintf_s(WCHAR* buf, size_t size, size_t count, const WCHAR* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	const int n = u16::vsnprintf_s(buf, size, count, fmt, ap);
	va_end(ap);
	return n;
}

// 字面量的类型必须是 WCHAR
static_assert(std::is_same_v<std::remove_cv_t<std::remove_reference_t<decltype(LTEXT("a")[0])>>, WCHAR>);
static_assert(std::is_same_v<std::remove_cv_t<decltype(LCHAR('a'))>, WCHAR>);
static_assert(sizeof(LTEXT("さくら")) == 4 * sizeof(WCHAR), "UTF-16 literal: 3 units + NUL");
static_assert(sizeof(LTEXT("😀")) == 3 * sizeof(WCHAR), "surrogate pair: 2 units + NUL");
static_assert(u16::strlen(LTEXT("abc")) == 3, "constexpr strlen");

static void test_basic()
{
	const WCHAR* s = LTEXT("Hello, さくら!");
	CHECK(u16::strlen(s) == 11);
	CHECK(u16::strnlen(s, 5) == 5);
	CHECK(u16::strcmp(LTEXT("abc"), LTEXT("abd")) < 0);
	CHECK(u16::strcmp(LTEXT("abc"), LTEXT("abc")) == 0);
	CHECK(u16::strcmp(LTEXT("\xFF21"), LTEXT("A")) > 0);              // 无符号比较
	CHECK(u16::strncmp(LTEXT("abcX"), LTEXT("abcY"), 3) == 0);
	CHECK(u16::stricmp(LTEXT("Sakura"), LTEXT("SAKURA")) == 0);
	CHECK(u16::strnicmp(LTEXT("SakuraX"), LTEXT("sakuraY"), 6) == 0);
	CHECK(u16::strchr(s, LCHAR('く')) == s + 8);
	CHECK(u16::strrchr(LTEXT("a/b/c"), LCHAR('/'))[1] == LCHAR('c'));
	CHECK(u16::strstr(s, LTEXT("さくら")) == s + 7);
	CHECK(u16::strstr(s, LTEXT("xyz")) == nullptr);
	CHECK(u16::strpbrk(s, LTEXT(",!")) == s + 5);
	CHECK(u16::strspn(LTEXT("  x"), LTEXT(" ")) == 2);

	WCHAR buf[32];
	u16::strcpy(buf, LTEXT("さくら"));
	u16::strcat(buf, LTEXT("エディタ"));
	CHECK(eq(buf, LTEXT("さくらエディタ")));
	u16::strncat(buf, LTEXT("123456"), 2);
	CHECK(eq(buf, LTEXT("さくらエディタ12")));

	WCHAR* dup = u16::strdup(LTEXT("dup"));
	CHECK(eq(dup, LTEXT("dup")));
	std::free(dup);

	WCHAR tok[] = LTEXT("a,,b, c");
	WCHAR* ctx = nullptr;
	const WCHAR* t1 = u16::strtok(tok, LTEXT(", "), &ctx);
	const WCHAR* t2 = u16::strtok<WCHAR>(nullptr, LTEXT(", "), &ctx);
	const WCHAR* t3 = u16::strtok<WCHAR>(nullptr, LTEXT(", "), &ctx);
	const WCHAR* t4 = u16::strtok<WCHAR>(nullptr, LTEXT(", "), &ctx);
	CHECK(t1 && eq(t1, LTEXT("a")));
	CHECK(t2 && eq(t2, LTEXT("b")));
	CHECK(t3 && eq(t3, LTEXT("c")));
	CHECK(t4 == nullptr);
}

static void test_number()
{
	CHECK(u16::atoi(LTEXT("  -123abc")) == -123);
	CHECK(u16::atoi(LTEXT("99999999999")) == INT_MAX);
	WCHAR* end = nullptr;
	CHECK(u16::strtol(LTEXT("0x1F;"), &end, 16) == 31 && *end == LCHAR(';'));
	CHECK(u16::strtol(LTEXT("１２"), &end, 10) == 0);                 // 全角数字不是数字（与 MSVC 相同）
	WCHAR buf[72];
	CHECK(eq(u16::itoa32(-42, buf, 10), LTEXT("-42")));
	CHECK(eq(u16::itoa32(-1, buf, 16), LTEXT("ffffffff")));
	CHECK(eq(u16::itoa32(255, buf, 2), LTEXT("11111111")));
	CHECK(eq(u16::itoa64(-1, buf, 16), LTEXT("ffffffffffffffff")));
	CHECK(u16::strtod(LTEXT("1.5e2"), &end) == 150.0);
}

static void test_format()
{
	WCHAR buf[128];
	CHECK(u16::format(buf, 128, LTEXT("%d-%05.1f-%s-%hs-%c|%-4s|%4s|"), 7, 3.14159, LTEXT("さくら"), "utf8:\xE6\xA1\x9C", LCHAR('桜'), LTEXT("ab"), LTEXT("cd")) > 0);
	CHECK(eq(buf, LTEXT("7-003.1-さくら-utf8:桜-桜|ab  |  cd|")));
	u16::format(buf, 128, LTEXT("%I64d %I64u %llx %zu %08X %p"), -1LL, 18446744073709551615ULL, 255ULL, size_t(9), 0xABCDu, static_cast<void*>(nullptr));
	CHECK(u16::strncmp(buf, LTEXT("-1 18446744073709551615 ff 9 0000ABCD "), 38) == 0);
	u16::format(buf, 128, LTEXT("[%*d][%-*d][%.2s][%%]"), 4, 1, 3, 2, LTEXT("xyz"));
	CHECK(eq(buf, LTEXT("[   1][2  ][xy][%]")));
	u16::format(buf, 128, LTEXT("%s"), static_cast<const WCHAR*>(nullptr));
	CHECK(eq(buf, LTEXT("(null)")));
	CHECK(u16::format(buf, 128, LTEXT("%n"), nullptr) == -1);

	// 切り詰め（_snwprintf_s と同じ規則）
	WCHAR small[6];
	CHECK(call_vsnprintf_s(small, 6, _TRUNCATE, LTEXT("%s"), LTEXT("abc")) == 3 && eq(small, LTEXT("abc")));
	CHECK(call_vsnprintf_s(small, 6, _TRUNCATE, LTEXT("%s"), LTEXT("abcdefgh")) == -1 && eq(small, LTEXT("abcde")));
	CHECK(call_vsnprintf_s(small, 6, 2, LTEXT("%s"), LTEXT("abcdefgh")) == -1 && eq(small, LTEXT("ab")));
}

static void test_utf()
{
	const WCHAR* s = LTEXT("A桜😀");
	const std::string u8 = u16::to_utf8(s);
	CHECK(u8 == "A\xE6\xA1\x9C\xF0\x9F\x98\x80");
	CHECK(u16::from_utf8(u8) == sakura::wstring(s));
	CHECK(u16::from_utf8("\xFF") == sakura::wstring(LTEXT("\xFFFD")));     // 不正なバイト
	CHECK(u16::from_utf8("\xE6\xA1") == sakura::wstring(LTEXT("\xFFFD"))); // 途中で終わる
	const WCHAR lone[] = { 0xD83D, 0 };                                    // 対のないサロゲート
	CHECK(u16::to_utf8(lone) == "\xEF\xBF\xBD");
	const std::wstring native = u16::to_native_wide(sakura::wstring_view(s));
	CHECK(native.size() == (sizeof(wchar_t) == 2 ? 4u : 3u));
	CHECK(u16::from_native_wide(native) == sakura::wstring(s));
	CHECK(u16::to_lower(LCHAR('Ä')) == LCHAR('ä') || u16::to_lower(LCHAR('Ä')) == LCHAR('Ä'));  // 依存于 locale
	CHECK(u16::to_lower(LCHAR('Q')) == LCHAR('q'));
}

#if defined(_WIN32)
// Windows：与 MSVC 运行库对比
static void test_compare_with_msvc()
{
	const WCHAR* cases[] = { L"", L"a", L"ABC", L"abc", L"abd", L"さくら", L"サクラ", L"\xFF21", L"a,b;c" };
	for (auto a : cases) {
		CHECK(u16::strlen(a) == ::wcslen(a));
		for (auto b : cases) {
			auto sign = [](int v) { return (v > 0) - (v < 0); };
			CHECK(sign(u16::strcmp(a, b)) == sign(::wcscmp(a, b)));
			CHECK(sign(u16::strncmp(a, b, 2)) == sign(::wcsncmp(a, b, 2)));
			CHECK((u16::strstr(a, b) ? u16::strstr(a, b) - a : -1) == (::wcsstr(a, b) ? ::wcsstr(a, b) - a : -1));
		}
	}
	WCHAR x[128], y[128];
	auto same = [&](int line) {
		CHECK(::wcscmp(x, y) == 0);
		if (::wcscmp(x, y) != 0) std::printf("  line %d: sakura=[%s] msvc=[%s]\n", line, u16::to_utf8(x).c_str(), u16::to_utf8(y).c_str());
	};
	u16::format(x, 128, L"%d|%5d|%-5d|%05d|%+d", 42, 42, -42, 42, 42);
	_snwprintf_s(y, 128, _TRUNCATE, L"%d|%5d|%-5d|%05d|%+d", 42, 42, -42, 42, 42);
	same(__LINE__);
	u16::format(x, 128, L"%x|%X|%#x|%o|%I64d|%llu|%zu", 255, 255, 255, 8, -1LL, 1ULL, size_t(3));
	_snwprintf_s(y, 128, _TRUNCATE, L"%x|%X|%#x|%o|%I64d|%llu|%zu", 255, 255, 255, 8, -1LL, 1ULL, size_t(3));
	same(__LINE__);
	u16::format(x, 128, L"%.3f|%e|%g|%8.2f", 1.5, 1.5, 1.5, -2.25);
	_snwprintf_s(y, 128, _TRUNCATE, L"%.3f|%e|%g|%8.2f", 1.5, 1.5, 1.5, -2.25);
	same(__LINE__);
	u16::format(x, 128, L"%s|%ls|%hs|%S|%c|%hc|%-3s|%.1s", L"w", L"l", "n", "S", L'c', 'h', L"a", L"xyz");
	_snwprintf_s(y, 128, _TRUNCATE, L"%s|%ls|%hs|%S|%c|%hc|%-3s|%.1s", L"w", L"l", "n", "S", L'c', 'h', L"a", L"xyz");
	same(__LINE__);
	WCHAR small1[6], small2[6];
	auto both = [&](size_t count, const WCHAR* s) {
		const int r1 = call_vsnprintf_s(small1, 6, count, L"%s", s);
		const int r2 = _snwprintf_s(small2, 6, count, L"%s", s);
		CHECK(r1 == r2);
		CHECK(::wcscmp(small1, small2) == 0);
	};
	both(_TRUNCATE, L"abc");
	both(_TRUNCATE, L"abcdefgh");
	both(2, L"abcdefgh");
	WCHAR i1[72], i2[72];
	for (int v : { 0, 1, -1, 42, -42, INT_MAX, INT_MIN }) for (int r : { 2, 8, 10, 16, 36 }) {
		CHECK(::wcscmp(u16::itoa32(v, i1, r), ::_itow(v, i2, r)) == 0);
	}
	for (const WCHAR* s : { L"12", L"-7x", L"  +3", L"99999999999", L"-99999999999", L"" }) {
		CHECK(u16::atoi(s) == ::_wtoi(s));
	}
}
#else
// macOS / Linux：直接调用与 MSVC 同名的 char16_t 重载
static void test_overloads()
{
	WCHAR buf[64];
	wcscpy(buf, LTEXT("さくら"));
	wcscat(buf, LTEXT("Editor"));
	CHECK(wcslen(buf) == 9);
	CHECK(wcscmp(buf, LTEXT("さくらEditor")) == 0);
	CHECK(_wcsicmp(buf, LTEXT("さくらEDITOR")) == 0);
	CHECK(wcschr(buf, LCHAR('E')) == buf + 3);
	CHECK(wcsstr(buf, LTEXT("dit")) == buf + 4);
	CHECK(_wtoi(LTEXT("123")) == 123);
	_itow(-5, buf, 10);
	CHECK(wcscmp(buf, LTEXT("-5")) == 0);
	CHECK(_swprintf(buf, LTEXT("%s=%d"), LTEXT("x"), 1) == 3 && wcscmp(buf, LTEXT("x=1")) == 0);
	CHECK(swprintf(buf, 3, LTEXT("%s"), LTEXT("long")) == -1);
	CHECK(_snwprintf_s(buf, 64, _TRUNCATE, LTEXT("%ls/%hs"), LTEXT("u16"), "u8") == 6 && wcscmp(buf, LTEXT("u16/u8")) == 0);
	CHECK(_scwprintf(LTEXT("%s%d"), LTEXT("abc"), 12345) == 8);
	WCHAR mem[4] = { 1, 2, 3, 4 };
	wmemset(mem, 9, 2);
	CHECK(mem[0] == 9 && mem[1] == 9 && mem[2] == 3);
	CHECK(wmemcmp(mem, mem, 4) == 0);
	sakura::wstring str = LTEXT("std::basic_string<WCHAR>");
	str += LTEXT(" OK");
	CHECK(str.size() == 27);
	CHECK(str.find(LTEXT("OK")) == 25);
}
#endif

int main()
{
	std::setlocale(LC_ALL, "");
	test_basic();
	test_number();
	test_format();
	test_utf();
#if defined(_WIN32)
	test_compare_with_msvc();
#else
	test_overloads();
#endif
	std::printf("%s: %d passed, %d failed (sizeof(WCHAR)=%zu, WCHAR is %s)\n",
		g_failed ? "NG" : "OK", g_passed, g_failed, sizeof(WCHAR), SAKURA_WCHAR_IS_CHAR16 ? "char16_t" : "wchar_t");
	return g_failed ? 1 : 0;
}
