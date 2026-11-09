/*! @file
	@brief Mac 版：代码页转换（macos/platform/src/codepage.cpp）的测试

	预期值是在 Windows 上实际调用 MultiByteToWideChar / WideCharToMultiByte 得到的结果。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include <string>
#include <vector>

namespace {

sakura::wstring ToWide(UINT cp, std::string_view s, DWORD flags = 0)
{
	const int n = MultiByteToWideChar(cp, flags, s.data(), int(s.size()), nullptr, 0);
	if (n <= 0) return LTEXT("<error>");
	sakura::wstring w(size_t(n), LCHAR('\0'));
	EXPECT_EQ(MultiByteToWideChar(cp, flags, s.data(), int(s.size()), w.data(), n), n);
	return w;
}

std::string ToMulti(UINT cp, sakura::wstring_view w, DWORD flags = 0, BOOL* used = nullptr)
{
	const int n = WideCharToMultiByte(cp, flags, w.data(), int(w.size()), nullptr, 0, nullptr, used);
	if (n <= 0) return "<error>";
	std::string s(size_t(n), '\0');
	EXPECT_EQ(WideCharToMultiByte(cp, flags, w.data(), int(w.size()), s.data(), n, nullptr, used), n);
	return s;
}

} // namespace

TEST(MacCodePage, AcpIsUtf8)
{
	EXPECT_EQ(GetACP(), UINT(CP_UTF8));
	EXPECT_EQ(GetOEMCP(), UINT(CP_UTF8));
	EXPECT_EQ(ToWide(CP_ACP, "\xE3\x81\x82"), LTEXT("あ"));
}

TEST(MacCodePage, ValidCodePages)
{
	for (UINT cp : { 437u, 850u, 874u, 932u, 936u, 949u, 950u, 1250u, 1251u, 1252u, 1253u, 1254u, 1255u, 1256u, 1257u, 1258u, 20932u, 28591u, 65001u }) {
		EXPECT_TRUE(IsValidCodePage(cp)) << cp;
	}
	EXPECT_FALSE(IsValidCodePage(12345));
	CPINFOEX info{};
	ASSERT_TRUE(GetCPInfoEx(932, 0, &info));
	EXPECT_EQ(info.MaxCharSize, 2u);
	EXPECT_EQ(info.DefaultChar[0], '?');
	EXPECT_EQ(info.UnicodeDefaultChar, 0x30FB);
	EXPECT_TRUE(IsDBCSLeadByteEx(932, 0x81));
	EXPECT_FALSE(IsDBCSLeadByteEx(932, 0x41));
	EXPECT_FALSE(IsDBCSLeadByteEx(932, 0xA1));   // 半角片假名
}

TEST(MacCodePage, EnumSystemCodePages)
{
	static std::vector<UINT> cps;
	cps.clear();
	EXPECT_TRUE(EnumSystemCodePages([](WCHAR* name) -> BOOL {
		cps.push_back(UINT(std::stoul(sakura::u16::to_utf8(name))));
		return TRUE;
	}, CP_INSTALLED));
	EXPECT_GT(cps.size(), 100u);
	EXPECT_NE(std::find(cps.begin(), cps.end(), 932u), cps.end());
	EXPECT_NE(std::find(cps.begin(), cps.end(), 65001u), cps.end());
}

TEST(MacCodePage, ShiftJis)
{
	EXPECT_EQ(ToWide(932, "\x82\xA0\x88\xA4\xB1"), LTEXT("あ愛ｱ"));
	EXPECT_EQ(ToMulti(932, LTEXT("あ愛ｱ")), "\x82\xA0\x88\xA4\xB1");
	// NEC 特殊字符、IBM 扩展字符（Windows 的 CP932 特有）
	EXPECT_EQ(ToWide(932, "\x87\x40"), LTEXT("①"));
	EXPECT_EQ(ToMulti(932, LTEXT("①")), "\x87\x40");
	EXPECT_EQ(ToMulti(932, LTEXT("≒")), "\x81\xE0");   // ≒ 在 CP932 中有 81E0 和 8790（NEC 特殊字符）两个位置，Unicode → CP932 时用 81E0
	// 全角波浪线 U+FF5E 是 CP932 的 8160；波浪号 U+301C 不在 CP932 中，best fit 也换到 8160
	BOOL used = FALSE;
	EXPECT_EQ(ToMulti(932, LTEXT("～"), 0, &used), "\x81\x60");
	EXPECT_FALSE(used);
	// 无法转换的字符变成替代字符 '?'，并设置 used
	used = FALSE;
	EXPECT_EQ(ToMulti(932, LTEXT("é"), WC_NO_BEST_FIT_CHARS, &used), "?");
	EXPECT_TRUE(used);
	// 不正确的第 2 字节
	EXPECT_EQ(MultiByteToWideChar(932, MB_ERR_INVALID_CHARS, "\x81\x20", 2, nullptr, 0), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_NO_UNICODE_TRANSLATION));
}

TEST(MacCodePage, Utf8)
{
	EXPECT_EQ(ToWide(CP_UTF8, "a\xF0\x9F\x98\x80"), LTEXT("a\U0001F600"));
	EXPECT_EQ(ToMulti(CP_UTF8, LTEXT("a\U0001F600")), "a\xF0\x9F\x98\x80");
	// 不正确序列的处理（预期值是 Windows 上的实际结果，由 macos/tools/codepage_dump 的 utf8-probe 取得）
	EXPECT_EQ(ToWide(CP_UTF8, "\xE3\x81" "A" "\x80"), LTEXT("\uFFFDA\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xED\xA0\x80"), LTEXT("\uFFFD\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xC0\xAF"), LTEXT("\uFFFD\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xF4\x90\x80\x80"), LTEXT("\uFFFD\uFFFD\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xE0\x80\x80"), LTEXT("\uFFFD\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xF0\x9F\x98"), LTEXT("\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xFF"), LTEXT("\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xC2"), LTEXT("\uFFFD"));
	EXPECT_EQ(ToWide(CP_UTF8, "\xE3\x81\x82\x80\x80"), LTEXT("あ\uFFFD\uFFFD"));
	EXPECT_EQ(MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, "\xC0\xAF", 2, nullptr, 0), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_NO_UNICODE_TRANSLATION));
	// 不成对的代理项变成 U+FFFD（EF BF BD）
	const WCHAR lone[] = { 0xD800, u'A' };
	EXPECT_EQ(ToMulti(CP_UTF8, sakura::wstring_view(lone, 2)), "\xEF\xBF\xBD" "A");
	EXPECT_EQ(WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, lone, 2, nullptr, 0, nullptr, nullptr), 0);
	// 指定 lpUsedDefaultChar 也能转换（Windows 上的实际结果）
	BOOL used = FALSE;
	EXPECT_EQ(WideCharToMultiByte(CP_UTF8, 0, lone, 2, nullptr, 0, nullptr, &used), 4);
}

TEST(MacCodePage, NullTerminatedAndBuffer)
{
	// 长度 -1 表示到结尾的 NUL 为止（NUL 也计入）
	EXPECT_EQ(MultiByteToWideChar(932, 0, "\x82\xA0", -1, nullptr, 0), 2);
	WCHAR buf[1];
	EXPECT_EQ(MultiByteToWideChar(932, 0, "ab", 2, buf, 1), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INSUFFICIENT_BUFFER));
}

TEST(MacCodePage, Latin1AndCyrillic)
{
	EXPECT_EQ(ToWide(1252, "\x80\xE9"), LTEXT("€é"));
	EXPECT_EQ(ToMulti(1252, LTEXT("€é")), "\x80\xE9");
	EXPECT_EQ(ToWide(1251, "\xC0\xFF"), LTEXT("Ая"));
	// best fit: Ā（U+0100）在 1252 中换成 A
	EXPECT_EQ(ToMulti(1252, LTEXT("Ā")), "A");
	EXPECT_EQ(ToMulti(1252, LTEXT("Ā"), WC_NO_BEST_FIT_CHARS), "?");
}
