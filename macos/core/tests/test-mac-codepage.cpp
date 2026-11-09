/*! @file
	@brief Mac 版：代码页转换（macos/platform/src/codepage.cpp）的测试

	预期值是在 Windows 上实际调用 MultiByteToWideChar / WideCharToMultiByte 得到的结果。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"

#include "charset/CCodePage.h"
#include "mem/CMemory.h"
#include "mem/CNativeW.h"

#include <cstring>
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

namespace {

sakura::wstring W(std::initializer_list<char16_t> units) { return sakura::wstring(units.begin(), units.end()); }

} // namespace

TEST(MacCodePage, Gb18030)
{
	// 期待值都是 Windows 上的探测结果（macos/tools/codepage_dump 的 gb18030-probe）
	EXPECT_EQ(ToWide(54936, "\xC4\xE3\xBA\xC3"), LTEXT("你好"));
	EXPECT_EQ(ToWide(54936, "\xA1\xA1", MB_ERR_INVALID_CHARS), W({ 0x3000 }));
	EXPECT_EQ(ToWide(54936, "\x81\x30\x81\x30", MB_ERR_INVALID_CHARS), W({ 0x0080 }));
	EXPECT_EQ(ToWide(54936, "\x84\x31\xA4\x39", MB_ERR_INVALID_CHARS), W({ 0xFFFF }));
	EXPECT_EQ(ToWide(54936, "\x94\x39\xFC\x36", MB_ERR_INVALID_CHARS), W({ 0xD83D, 0xDE00 }));
	EXPECT_EQ(ToWide(54936, "\x90\x30\x81\x30", MB_ERR_INVALID_CHARS), W({ 0xD800, 0xDC00 }));
	EXPECT_EQ(ToWide(54936, "\xE3\x32\x9A\x35", MB_ERR_INVALID_CHARS), W({ 0xDBFF, 0xDFFF }));
	// 格式正确但没有对应的 4 字节序列：'?'，严格模式也不算错误
	EXPECT_EQ(ToWide(54936, "\x84\x31\xA5\x30", MB_ERR_INVALID_CHARS), W({ 0x003F }));
	EXPECT_EQ(ToWide(54936, "\xE3\x32\x9A\x36", MB_ERR_INVALID_CHARS), W({ 0x003F }));
	EXPECT_EQ(ToWide(54936, "\xFE\x39\xFE\x39", MB_ERR_INVALID_CHARS), W({ 0x003F }));
	// 不完整、不正确的序列
	EXPECT_EQ(ToWide(54936, "\x81\x30"), W({ 0x003F, 0x0030 }));
	EXPECT_EQ(ToWide(54936, "\x81\x30\x81"), W({ 0x003F, 0x0030, 0x003F }));
	EXPECT_EQ(ToWide(54936, "\x81\x30\x41\x30"), W({ 0x003F, 0x0030, 0x0041, 0x0030 }));
	EXPECT_EQ(ToWide(54936, "\x81\x30\x81\x41"), W({ 0x003F, 0x0030, 0x4E04 }));
	EXPECT_EQ(ToWide(54936, "\x80"), W({ 0x003F }));
	EXPECT_EQ(ToWide(54936, "\xFF"), W({ 0x003F }));
	EXPECT_EQ(ToWide(54936, "\x81\x7F"), W({ 0x003F, 0x007F }));
	for (const char* bad : { "\x81\x30", "\x80", "\xFF", "\x81\x7F" }) {
		SetLastError(0);
		EXPECT_EQ(MultiByteToWideChar(54936, MB_ERR_INVALID_CHARS, bad, int(std::strlen(bad)), nullptr, 0), 0) << bad;
		EXPECT_EQ(GetLastError(), DWORD(ERROR_NO_UNICODE_TRANSLATION));
	}

	EXPECT_EQ(ToMulti(54936, LTEXT("你好\U0001F600")), "\xC4\xE3\xBA\xC3\x94\x39\xFC\x36");
	EXPECT_EQ(ToMulti(54936, W({ 0xDBFF, 0xDFFF })), "\xE3\x32\x9A\x35");
	EXPECT_EQ(ToMulti(54936, W({ 0xD800 })), "?");
	EXPECT_EQ(ToMulti(54936, W({ 0xE5E5 })), "\xA3\xA0");
	EXPECT_EQ(ToMulti(54936, W({ 0x0080 })), "\x81\x30\x81\x30");
	EXPECT_EQ(ToMulti(54936, W({ 0xFFFF })), "\x84\x31\xA4\x39");

	CPINFOEX info{};
	ASSERT_TRUE(GetCPInfoEx(54936, 0, &info));
	EXPECT_EQ(info.MaxCharSize, 4u);
	EXPECT_EQ(info.UnicodeDefaultChar, 0x003F);

	// 参数检查（param-probe）：lpUsedDefaultChar / lpDefaultChar 是 87，WC_NO_BEST_FIT_CHARS 是 1004，WC_ERR_INVALID_CHARS 可以用
	char out[8];
	BOOL used = FALSE;
	SetLastError(0);
	EXPECT_EQ(WideCharToMultiByte(54936, 0, LTEXT("a"), 1, out, 8, nullptr, &used), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_PARAMETER));
	SetLastError(0);
	EXPECT_EQ(WideCharToMultiByte(54936, 0, LTEXT("a"), 1, out, 8, "*", nullptr), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_PARAMETER));
	SetLastError(0);
	EXPECT_EQ(WideCharToMultiByte(54936, WC_NO_BEST_FIT_CHARS, LTEXT("a"), 1, out, 8, nullptr, nullptr), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_FLAGS));
	EXPECT_EQ(ToMulti(54936, LTEXT("a"), WC_ERR_INVALID_CHARS), "a");
	EXPECT_EQ(ToWide(54936, "a", MB_ERR_INVALID_CHARS), LTEXT("a"));
	// 932 不接受 WC_ERR_INVALID_CHARS
	SetLastError(0);
	EXPECT_EQ(WideCharToMultiByte(932, WC_ERR_INVALID_CHARS, LTEXT("a"), 1, out, 8, nullptr, nullptr), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_FLAGS));
}

TEST(MacCodePage, Utf7)
{
	// 期待值都是 Windows 上的探测结果（utf7-probe）
	EXPECT_EQ(ToWide(CP_UTF7, "+MEI-"), LTEXT("あ"));
	EXPECT_EQ(ToWide(CP_UTF7, "+MEI"), LTEXT("あ"));
	EXPECT_EQ(ToWide(CP_UTF7, "+MEI-x"), LTEXT("あx"));
	EXPECT_EQ(ToWide(CP_UTF7, "+MEIw"), LTEXT("あ"));
	EXPECT_EQ(ToWide(CP_UTF7, "+MEIwRA"), LTEXT("あい"));
	EXPECT_EQ(ToWide(CP_UTF7, "+-"), LTEXT("+"));
	EXPECT_EQ(ToWide(CP_UTF7, "+"), LTEXT("<error>"));
	EXPECT_EQ(ToWide(CP_UTF7, "a+MEI.b"), LTEXT("aあ.b"));
	EXPECT_EQ(ToWide(CP_UTF7, "+ZeVnLIqe-"), LTEXT("日本語"));
	EXPECT_EQ(ToWide(CP_UTF7, "+!"), LTEXT("!"));
	EXPECT_EQ(ToWide(CP_UTF7, "\x80"), W({ 0x0080 }));
	EXPECT_EQ(ToWide(CP_UTF7, "+MEIAQg-"), LTEXT("あB"));
	EXPECT_EQ(ToWide(CP_UTF7, "+2D3eAA-"), LTEXT("\U0001F600"));
	EXPECT_EQ(ToWide(CP_UTF7, "+2D0-"), W({ 0xD83D }));
	EXPECT_EQ(ToWide(CP_UTF7, "~\\"), LTEXT("~\\"));
	EXPECT_EQ(ToWide(CP_UTF7, "+AGE-"), LTEXT("a"));
	SetLastError(0);
	EXPECT_EQ(MultiByteToWideChar(CP_UTF7, MB_ERR_INVALID_CHARS, "+MEI-", 5, nullptr, 0), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_FLAGS));

	EXPECT_EQ(ToMulti(CP_UTF7, LTEXT("あa")), "+MEI-a");
	EXPECT_EQ(ToMulti(CP_UTF7, LTEXT("a+b")), "a+-b");
	EXPECT_EQ(ToMulti(CP_UTF7, LTEXT("あ-")), "+MEI--");
	EXPECT_EQ(ToMulti(CP_UTF7, LTEXT("ああ")), "+MEIwQg-");
	EXPECT_EQ(ToMulti(CP_UTF7, LTEXT("あ.")), "+MEI-.");
	EXPECT_EQ(ToMulti(CP_UTF7, LTEXT("\U0001F600")), "+2D3eAA-");
	EXPECT_EQ(ToMulti(CP_UTF7, W({ 0xD800 })), "+2AA-");
	EXPECT_EQ(ToMulti(CP_UTF7, LTEXT("あ い")), "+MEI- +MEQ-");
	// ASCII 0x00〜0x7F 全部和 Windows 的结果对照：直接输出的字符以外是 "+AA?-" 形式
	const std::string direct = std::string(1, '\0') + "\t\n\r '(),-./0123456789:?ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	for (char16_t c = 0; c < 0x80; ++c) {
		const std::string got = ToMulti(CP_UTF7, sakura::wstring(1, c));
		if (c == u'+') {
			EXPECT_EQ(got, "+-");
		} else if (direct.find(char(c)) != std::string::npos) {
			EXPECT_EQ(got, std::string(1, char(c))) << int(c);
		} else {
			const char* b64 = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
			const std::string want = std::string("+A") + b64[c >> 4] + b64[(c & 0xF) << 2] + "-";
			EXPECT_EQ(got, want) << int(c);
		}
	}
	EXPECT_EQ(ToMulti(CP_UTF7, W({ 0x005B })), "+AFs-");
	EXPECT_EQ(ToMulti(CP_UTF7, W({ 0x007F })), "+AH8-");
	char out[8];
	BOOL used = FALSE;
	SetLastError(0);
	EXPECT_EQ(WideCharToMultiByte(CP_UTF7, 0, LTEXT("a"), 1, out, 8, nullptr, &used), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_PARAMETER));
	SetLastError(0);
	EXPECT_EQ(WideCharToMultiByte(CP_UTF7, 0, LTEXT("a"), 1, out, 8, "*", nullptr), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_PARAMETER));
	CPINFOEX info{};
	ASSERT_TRUE(GetCPInfoEx(CP_UTF7, 0, &info));
	EXPECT_EQ(info.MaxCharSize, 5u);
	EXPECT_EQ(info.UnicodeDefaultChar, 0xFFFD);
	SetLastError(0);
	EXPECT_EQ(WideCharToMultiByte(CP_UTF7, WC_ERR_INVALID_CHARS, LTEXT("a"), 1, nullptr, 0, nullptr, nullptr), 0);
	EXPECT_EQ(GetLastError(), DWORD(ERROR_INVALID_FLAGS));
}

TEST(MacCodePage, IconvCodePages)
{
	// ISO-2022 系、HZ 由 iconv 实现
	EXPECT_EQ(ToMulti(50220, LTEXT("aあ")), "a\x1B$B$\"\x1B(B");
	EXPECT_EQ(ToWide(50220, "a\x1B$B$\"\x1B(B"), LTEXT("aあ"));
}

// 原版没有直接测试 CCodePage 类（任意代码页的读写）的用例，Mac 版在这里补上
namespace {

EConvertResult CpToUni(int cp, std::string_view bytes, sakura::wstring& out)
{
	CMemory src(bytes.data(), int(bytes.size()));
	CNativeW dst;
	const auto r = CCodePage::CPToUnicode(src, &dst, cp);
	out.assign(dst.GetStringPtr(), size_t(dst.GetStringLength()));
	return r;
}

EConvertResult UniToCp(int cp, sakura::wstring_view text, std::string& out)
{
	CNativeW src(text.data(), text.size());
	CMemory dst;
	const auto r = CCodePage::UnicodeToCP(src, &dst, cp);
	out.assign(reinterpret_cast<const char*>(dst.GetRawPtr()), size_t(dst.GetRawLength()));
	return r;
}

} // namespace

TEST(MacCodePage, CCodePageClass)
{
	sakura::wstring w;
	std::string m;
	EXPECT_EQ(CpToUni(932, "\x82\xA0\x8A\xBF", w), RESULT_COMPLETE);
	EXPECT_EQ(w, LTEXT("あ漢"));
	EXPECT_EQ(UniToCp(932, LTEXT("あ漢"), m), RESULT_COMPLETE);
	EXPECT_EQ(m, "\x82\xA0\x8A\xBF");
	EXPECT_EQ(CpToUni(54936, "\xC4\xE3\x94\x39\xFC\x36", w), RESULT_COMPLETE);
	EXPECT_EQ(w, LTEXT("你\U0001F600"));
	EXPECT_EQ(UniToCp(1252, LTEXT("caf\u00E9"), m), RESULT_COMPLETE);
	EXPECT_EQ(m, "caf\xE9");
	EXPECT_EQ(UniToCp(CP_UTF7, LTEXT("あa"), m), RESULT_COMPLETE);
	EXPECT_EQ(m, "+MEI-a");
	// CODE_CPACP は Mac 版では UTF-8（GetACP() == 65001）
	EXPECT_EQ(CpToUni(CODE_CPACP, "\xE3\x81\x82", w), RESULT_COMPLETE);
	EXPECT_EQ(w, LTEXT("あ"));
	// 独自実装の UTF-32
	EXPECT_EQ(UniToCp(12000, LTEXT("a"), m), RESULT_COMPLETE);
	EXPECT_EQ(m, std::string("a\0\0\0", 4));

	// 代码页一览：系统代码页 + UTF-32，按代码页排序
	const auto& list = CCodePage::GetCodePageList();
	auto has = [&](int cp) { return std::any_of(list.begin(), list.end(), [&](const auto& e) { return e.first == cp; }); };
	for (int cp : { 437, 932, 936, 949, 950, 1252, 54936, 65000, 65001, 12000, 12001, 50220 }) EXPECT_TRUE(has(cp)) << cp;
	EXPECT_TRUE(std::is_sorted(list.begin(), list.end(), [](const auto& a, const auto& b) { return a.first < b.first; }));
	for (const auto& e : list) EXPECT_FALSE(e.second.empty()) << e.first;
}
