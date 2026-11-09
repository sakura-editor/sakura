/*! @file
	@brief Windows 的代码页转换表导出工具（只在 Windows 上编译运行）

	对 Windows 上安装的每个单字节/双字节代码页，调用真正的 MultiByteToWideChar 和
	WideCharToMultiByte，把结果逐字符记录下来，写成一个二进制文件。
	Mac 版用这个文件实现与 Windows 完全一致的代码页转换（含 best fit 和替代字符）。

	用法: dump_codepages.exe <输出文件> [代码页一览.tsv]
	数据格式见 macos/platform/src/codepage_table.h。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#define NOMINMAX
#include <Windows.h>

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <set>
#include <string>
#include <vector>

namespace {

std::vector<UINT> g_codePages;

BOOL CALLBACK EnumProc(LPWSTR name)
{
	g_codePages.push_back(static_cast<UINT>(_wtoi(name)));
	return TRUE;
}

struct Writer {
	std::vector<uint8_t> buf;
	void u8(uint32_t v) { buf.push_back(static_cast<uint8_t>(v)); }
	void u16(uint32_t v) { u8(v & 0xFF); u8((v >> 8) & 0xFF); }
	void u32(uint32_t v) { u16(v & 0xFFFF); u16(v >> 16); }
};

// UTF 系、ISO-2022 系、GB18030 等状态相关或多于 2 字节的代码页由 Mac 版代码直接实现或不支持
bool IsSkipped(UINT cp)
{
	static const std::set<UINT> skip = {
		42, 1200, 1201, 12000, 12001, 65000, 65001,
		50220, 50221, 50222, 50225, 50227, 50229, 52936,
		57002, 57003, 57004, 57005, 57006, 57007, 57008, 57009, 57010, 57011,
	};
	return skip.contains(cp);
}

//! 一项 MultiByteToWideChar 的结果
void WriteMbEntry(Writer& w, UINT cp, const char* src, int len, const WCHAR* aloneLead, int aloneLeadLen, const WCHAR* aloneTrail, int aloneTrailLen)
{
	WCHAR out[8] = {};
	const int n = ::MultiByteToWideChar(cp, 0, src, len, out, 8);
	WCHAR tmp[8] = {};
	const bool strictOk = ::MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, src, len, tmp, 8) > 0;
	uint32_t flags = strictOk ? 1 : 0;
	// 2 字节输入的结果等于“前导字节单独 + 后续字节单独”时，视为只消耗了前导字节
	if (len == 2 && aloneLead && n == aloneLeadLen + aloneTrailLen
		&& std::wstring(out, n) == std::wstring(aloneLead, aloneLeadLen) + std::wstring(aloneTrail, aloneTrailLen)) {
		flags |= 2;
	}
	const int stored = n < 0 ? 0 : (n > 3 ? 3 : n);
	w.u8(flags);
	w.u8(stored);
	for (int i = 0; i < 3; ++i) w.u16(i < stored ? out[i] : 0);
}

//! WideCharToMultiByte 的包装。GB18030（54936）和 UTF-8 一样不接受 lpUsedDefaultChar 和 WC_NO_BEST_FIT_CHARS，
//! 所以不传这两个，变成替代字符 '?' 时自己判断 used（GB18030 能表示所有 Unicode，没有“最适合”映射）
int CallWc(UINT cp, DWORD flags, const WCHAR* src, int len, char* out, int cap, BOOL* used)
{
	if (cp != 54936) return ::WideCharToMultiByte(cp, flags, src, len, out, cap, nullptr, used);
	const int n = ::WideCharToMultiByte(cp, 0, src, len, out, cap, nullptr, nullptr);
	*used = (n == 1 && out[0] == '?' && !(len == 1 && src[0] == L'?')) ? TRUE : FALSE;
	return n;
}

void WriteWcResult(Writer& w, UINT cp, const WCHAR* src, int len)
{
	char out[8] = {};
	BOOL used = FALSE;
	const int n = CallWc(cp, 0, src, len, out, 8, &used);
	const int stored = n < 0 ? 0 : (n > 4 ? 4 : n);
	w.u8(stored);
	for (int i = 0; i < 4; ++i) w.u8(i < stored ? static_cast<uint8_t>(out[i]) : 0);
	w.u8(used ? 1 : 0);
}

//! WideCharToMultiByte 的结果一览（变成替代字符的不写）
void WriteWcTable(Writer& w, UINT cp, DWORD flags)
{
	std::vector<uint8_t> entries;
	uint32_t count = 0;
	for (uint32_t u = 0; u <= 0xFFFF; ++u) {
		if (0xD800 <= u && u <= 0xDFFF) continue;
		const WCHAR wc = static_cast<WCHAR>(u);
		char out[8] = {};
		BOOL used = FALSE;
		const int n = CallWc(cp, flags, &wc, 1, out, 8, &used);
		if (n <= 0 || used || n > 4) continue;
		entries.push_back(static_cast<uint8_t>(u & 0xFF)); entries.push_back(static_cast<uint8_t>(u >> 8));
		entries.push_back(static_cast<uint8_t>(n));
		for (int i = 0; i < 4; ++i) entries.push_back(i < n ? static_cast<uint8_t>(out[i]) : 0);
		++count;
	}
	w.u32(count);
	w.buf.insert(w.buf.end(), entries.begin(), entries.end());
}

//! UTF-8 的转换规则（不正确序列的处理等）。Mac 版的实现按这里的结果核对
void ProbeUtf8()
{
	const char* inputs[] = {
		"a\xF0\x9F\x98\x80", "\xE3\x81" "A" "\x80", "\xED\xA0\x80", "\xC0\xAF", "\xF4\x90\x80\x80",
		"\xE0\x80\x80", "\xF0\x9F\x98", "\xFF", "\xC2", "\xE3\x81\x82\x80\x80",
	};
	for (const char* in : inputs) {
		const int len = static_cast<int>(strlen(in));
		WCHAR out[32] = {};
		const int n = ::MultiByteToWideChar(CP_UTF8, 0, in, len, out, 32);
		const int strict = ::MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, in, len, nullptr, 0);
		wprintf(L"utf8-probe MB");
		for (int i = 0; i < len; ++i) wprintf(L" %02X", static_cast<unsigned char>(in[i]));
		wprintf(L" ->");
		for (int i = 0; i < n; ++i) wprintf(L" %04X", out[i]);
		wprintf(L" strict=%d err=%lu\n", strict, strict ? 0 : ::GetLastError());
	}
	const WCHAR lone[] = { 0xD800, L'A', 0xDC00 };
	char mb[16] = {};
	const int n = ::WideCharToMultiByte(CP_UTF8, 0, lone, 3, mb, 16, nullptr, nullptr);
	wprintf(L"utf8-probe WC D800 0041 DC00 ->");
	for (int i = 0; i < n; ++i) wprintf(L" %02X", static_cast<unsigned char>(mb[i]));
	BOOL used = FALSE;
	const int e1 = ::WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, lone, 3, nullptr, 0, nullptr, nullptr);
	const DWORD err1 = ::GetLastError();
	const int e2 = ::WideCharToMultiByte(CP_UTF8, 0, lone, 3, nullptr, 0, nullptr, &used);
	const DWORD err2 = ::GetLastError();
	wprintf(L" errInvalid=%d(%lu) usedDefault=%d(%lu)\n", e1, err1, e2, err2);
}

void PrintMb(const wchar_t* tag, UINT cp, const char* in, int len)
{
	WCHAR out[64] = {};
	const int n = ::MultiByteToWideChar(cp, 0, in, len, out, 64);
	const int strict = ::MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, in, len, nullptr, 0);
	const DWORD err = strict ? 0 : ::GetLastError();
	wprintf(L"%ls MB", tag);
	for (int i = 0; i < len; ++i) wprintf(L" %02X", static_cast<unsigned char>(in[i]));
	wprintf(L" ->");
	for (int i = 0; i < n; ++i) wprintf(L" %04X", out[i]);
	wprintf(L" strict=%d err=%lu\n", strict, err);
}

void PrintWc(const wchar_t* tag, UINT cp, const WCHAR* in, int len)
{
	char out[64] = {};
	const int n = ::WideCharToMultiByte(cp, 0, in, len, out, 64, nullptr, nullptr);
	wprintf(L"%ls WC", tag);
	for (int i = 0; i < len; ++i) wprintf(L" %04X", in[i]);
	wprintf(L" ->");
	for (int i = 0; i < n; ++i) wprintf(L" %02X", static_cast<unsigned char>(out[i]));
	wprintf(L" n=%d err=%lu\n", n, n ? 0 : ::GetLastError());
}

//! UTF-7 和 GB18030 的规则。Mac 版的实现按这里的结果核对
void ProbeUtf7AndGb18030()
{
	// 参数检查：哪些代码页、哪些标志接受 lpUsedDefaultChar / lpDefaultChar
	for (UINT cp : { 54936u, 65000u, 932u }) {
		const WCHAR wa[] = { L'a' };
		char out[8];
		BOOL used = FALSE;
		for (DWORD fl : { DWORD(0), DWORD(WC_NO_BEST_FIT_CHARS), DWORD(WC_ERR_INVALID_CHARS) }) {
			SetLastError(0);
			int n1 = ::WideCharToMultiByte(cp, fl, wa, 1, out, 8, nullptr, nullptr); DWORD e1 = GetLastError();
			SetLastError(0);
			int n2 = ::WideCharToMultiByte(cp, fl, wa, 1, out, 8, nullptr, &used); DWORD e2 = GetLastError();
			SetLastError(0);
			int n3 = ::WideCharToMultiByte(cp, fl, wa, 1, out, 8, "*", nullptr); DWORD e3 = GetLastError();
			wprintf(L"param-probe cp=%u flags=%lX plain=%d/%lu used=%d/%lu default=%d/%lu\n", cp, fl, n1, e1, n2, e2, n3, e3);
		}
		SetLastError(0);
		int m = ::MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, "a", 1, nullptr, 0);
		wprintf(L"param-probe cp=%u MB_ERR_INVALID_CHARS=%d/%lu\n", cp, m, GetLastError());
		CPINFOEXW info{};
		::GetCPInfoExW(cp, 0, &info);
		wprintf(L"param-probe cp=%u MaxCharSize=%u DefaultChar=%02X%02X UnicodeDefaultChar=%04X\n", cp, info.MaxCharSize, info.DefaultChar[0], info.DefaultChar[1], info.UnicodeDefaultChar);
	}
	for (WCHAR c = 0; c < 0x80; ++c) PrintWc(L"utf7-probe", CP_UTF7, &c, 1);
	const WCHAR w1[] = { 0x3042, L'a' }, w2[] = { L'a', L'+', L'b' }, w3[] = { 0x3042, L'-' }, w4[] = { 0x3042, 0x3042 },
		w5[] = { 0x3042, L'.' }, w6[] = { 0xD83D, 0xDE00 }, w7[] = { 0xD800 }, w8[] = { 0x3042, L' ', 0x3044 };
	PrintWc(L"utf7-probe", CP_UTF7, w1, 2); PrintWc(L"utf7-probe", CP_UTF7, w2, 3); PrintWc(L"utf7-probe", CP_UTF7, w3, 2);
	PrintWc(L"utf7-probe", CP_UTF7, w4, 2); PrintWc(L"utf7-probe", CP_UTF7, w5, 2); PrintWc(L"utf7-probe", CP_UTF7, w6, 2);
	PrintWc(L"utf7-probe", CP_UTF7, w7, 1); PrintWc(L"utf7-probe", CP_UTF7, w8, 3);
	const char* mb[] = { "+MEI-", "+MEI", "+MEI-x", "+MEIw", "+-", "+", "a+MEI.b", "+ZeVnLIqe-", "+!", "\x80", "+MEIAQg-", "+2D3eAA-", "+2D0-", "~\\", "+AGE-", "+MEIwRA" };
	for (const char* m : mb) PrintMb(L"utf7-probe", CP_UTF7, m, static_cast<int>(strlen(m)));

	const char* gb[] = { "\x81\x30\x81\x30", "\x94\x39\xFC\x36", "\x90\x30\x81\x30", "\x84\x31\xA4\x39", "\x84\x31\xA5\x30",
		"\xE3\x32\x9A\x35", "\xE3\x32\x9A\x36", "\xFE\x39\xFE\x39", "\x81\x30", "\x81\x30\x81", "\x81\x30\x41\x30", "\x81\x30\x81\x41", "\x80", "\xFF", "\x81\x7F", "\xA1\xA1" };
	for (const char* m : gb) PrintMb(L"gb18030-probe", 54936, m, static_cast<int>(strlen(m)));
	const WCHAR g1[] = { 0xD83D, 0xDE00 }, g2[] = { 0xDBFF, 0xDFFF }, g3[] = { 0xD800 }, g4[] = { 0xE5E5 }, g5[] = { 0x0080 }, g6[] = { 0xFFFF };
	PrintWc(L"gb18030-probe", 54936, g1, 2); PrintWc(L"gb18030-probe", 54936, g2, 2); PrintWc(L"gb18030-probe", 54936, g3, 1);
	PrintWc(L"gb18030-probe", 54936, g4, 1); PrintWc(L"gb18030-probe", 54936, g5, 1); PrintWc(L"gb18030-probe", 54936, g6, 1);
}

} // namespace

int wmain(int argc, wchar_t** argv)
{
	if (argc < 2) {
		fwprintf(stderr, L"usage: dump_codepages <out.bin> [list.tsv]\n");
		return 2;
	}
	ProbeUtf8();
	ProbeUtf7AndGb18030();
	::EnumSystemCodePagesW(EnumProc, CP_INSTALLED);
	std::set<UINT> sorted(g_codePages.begin(), g_codePages.end());

	Writer w;
	w.buf = { 'S', 'K', 'C', 'P' };
	w.u32(2);           // version（2: WideCharToMultiByte 的结果最多 4 字节，含 GB18030）
	const size_t countPos = w.buf.size();
	w.u32(0);           // count（最后再写入）

	FILE* list = nullptr;
	if (argc >= 3) _wfopen_s(&list, argv[2], L"w, ccs=UTF-8");

	uint32_t count = 0;
	for (UINT cp : sorted) {
		CPINFOEXW info{};
		if (!::GetCPInfoExW(cp, 0, &info)) continue;
		const bool skipped = IsSkipped(cp) || (info.MaxCharSize > 2 && cp != 54936);
		if (list) fwprintf(list, L"%u\t%u\t%ls\t%ls\n", cp, info.MaxCharSize, skipped ? L"skip" : L"table", info.CodePageName);
		if (skipped) continue;

		w.u32(cp);
		w.u8(info.MaxCharSize);
		w.u8(info.DefaultChar[0]);
		w.u8(info.DefaultChar[1]);
		w.u16(info.UnicodeDefaultChar);
		for (int i = 0; i < MAX_LEADBYTES; ++i) w.u8(info.LeadByte[i]);
		const std::wstring name = info.CodePageName;
		w.u16(static_cast<uint32_t>(name.size()));
		for (WCHAR c : name) w.u16(c);

		// 代理项的处理（成对、只有高位、只有低位）
		const WCHAR pair[] = { 0xD83D, 0xDE00 };
		const WCHAR hi[] = { 0xD83D };
		const WCHAR lo[] = { 0xDE00 };
		WriteWcResult(w, cp, pair, 2);
		WriteWcResult(w, cp, hi, 1);
		WriteWcResult(w, cp, lo, 1);

		// 1 字节输入
		WCHAR single[256][8] = {};
		int singleLen[256] = {};
		for (int b = 0; b < 256; ++b) {
			const char c = static_cast<char>(b);
			singleLen[b] = ::MultiByteToWideChar(cp, 0, &c, 1, single[b], 8);
			if (singleLen[b] < 0) singleLen[b] = 0;
			WriteMbEntry(w, cp, &c, 1, nullptr, 0, nullptr, 0);
		}
		// 2 字节输入（按前导字节）
		if (info.MaxCharSize == 2 || cp == 54936) {
			for (int lead = 0; lead < 256; ++lead) {
				// GB18030 的 CPINFO 没有前导字节范围，直接用 81〜FE（4 字节字符由 WideCharToMultiByte 的表反推）
				const bool isLead = (cp == 54936) ? (0x81 <= lead && lead <= 0xFE) : ::IsDBCSLeadByteEx(cp, static_cast<BYTE>(lead));
				if (!isLead) continue;
				for (int trail = 0; trail < 256; ++trail) {
					const char src[2] = { static_cast<char>(lead), static_cast<char>(trail) };
					WriteMbEntry(w, cp, src, 2, single[lead], singleLen[lead], single[trail], singleLen[trail]);
				}
			}
		}
		WriteWcTable(w, cp, 0);
		WriteWcTable(w, cp, WC_NO_BEST_FIT_CHARS);
		++count;
	}
	w.buf[countPos + 0] = count & 0xFF;
	w.buf[countPos + 1] = (count >> 8) & 0xFF;
	w.buf[countPos + 2] = (count >> 16) & 0xFF;
	w.buf[countPos + 3] = (count >> 24) & 0xFF;
	if (list) fclose(list);

	FILE* fp = nullptr;
	if (_wfopen_s(&fp, argv[1], L"wb") != 0 || !fp) return 1;
	fwrite(w.buf.data(), 1, w.buf.size(), fp);
	fclose(fp);
	fwprintf(stdout, L"%u code pages, %zu bytes\n", count, w.buf.size());
	return 0;
}
