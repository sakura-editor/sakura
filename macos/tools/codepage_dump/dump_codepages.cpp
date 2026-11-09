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
		50220, 50221, 50222, 50225, 50227, 50229, 52936, 54936,
		57002, 57003, 57004, 57005, 57006, 57007, 57008, 57009, 57010, 57011,
	};
	return skip.contains(cp);
}

//! MultiByteToWideChar の結果 1 件
void WriteMbEntry(Writer& w, UINT cp, const char* src, int len, const WCHAR* aloneLead, int aloneLeadLen, const WCHAR* aloneTrail, int aloneTrailLen)
{
	WCHAR out[8] = {};
	const int n = ::MultiByteToWideChar(cp, 0, src, len, out, 8);
	WCHAR tmp[8] = {};
	const bool strictOk = ::MultiByteToWideChar(cp, MB_ERR_INVALID_CHARS, src, len, tmp, 8) > 0;
	uint32_t flags = strictOk ? 1 : 0;
	// 2 バイト入力で、先行バイト単独 + 後続バイト単独 と同じ結果なら「先行バイトだけ消費した」
	if (len == 2 && aloneLead && n == aloneLeadLen + aloneTrailLen
		&& std::wstring(out, n) == std::wstring(aloneLead, aloneLeadLen) + std::wstring(aloneTrail, aloneTrailLen)) {
		flags |= 2;
	}
	const int stored = n < 0 ? 0 : (n > 3 ? 3 : n);
	w.u8(flags);
	w.u8(stored);
	for (int i = 0; i < 3; ++i) w.u16(i < stored ? out[i] : 0);
}

void WriteWcResult(Writer& w, UINT cp, const WCHAR* src, int len)
{
	char out[8] = {};
	BOOL used = FALSE;
	const int n = ::WideCharToMultiByte(cp, 0, src, len, out, 8, nullptr, &used);
	const int stored = n < 0 ? 0 : (n > 4 ? 4 : n);
	w.u8(stored);
	for (int i = 0; i < 4; ++i) w.u8(i < stored ? static_cast<uint8_t>(out[i]) : 0);
	w.u8(used ? 1 : 0);
}

//! WideCharToMultiByte の一覧（替代字符になったものは書かない）
void WriteWcTable(Writer& w, UINT cp, DWORD flags)
{
	std::vector<uint8_t> entries;
	uint32_t count = 0;
	for (uint32_t u = 0; u <= 0xFFFF; ++u) {
		if (0xD800 <= u && u <= 0xDFFF) continue;
		const WCHAR wc = static_cast<WCHAR>(u);
		char out[8] = {};
		BOOL used = FALSE;
		const int n = ::WideCharToMultiByte(cp, flags, &wc, 1, out, 8, nullptr, &used);
		if (n <= 0 || used || n > 2) continue;
		entries.push_back(static_cast<uint8_t>(u & 0xFF)); entries.push_back(static_cast<uint8_t>(u >> 8));
		entries.push_back(static_cast<uint8_t>(n));
		entries.push_back(static_cast<uint8_t>(out[0]));
		entries.push_back(n == 2 ? static_cast<uint8_t>(out[1]) : 0);
		++count;
	}
	w.u32(count);
	w.buf.insert(w.buf.end(), entries.begin(), entries.end());
}

} // namespace

int wmain(int argc, wchar_t** argv)
{
	if (argc < 2) {
		fwprintf(stderr, L"usage: dump_codepages <out.bin> [list.tsv]\n");
		return 2;
	}
	::EnumSystemCodePagesW(EnumProc, CP_INSTALLED);
	std::set<UINT> sorted(g_codePages.begin(), g_codePages.end());

	Writer w;
	w.buf = { 'S', 'K', 'C', 'P' };
	w.u32(1);           // version
	const size_t countPos = w.buf.size();
	w.u32(0);           // count（後で書き換える）

	FILE* list = nullptr;
	if (argc >= 3) _wfopen_s(&list, argv[2], L"w, ccs=UTF-8");

	uint32_t count = 0;
	for (UINT cp : sorted) {
		CPINFOEXW info{};
		if (!::GetCPInfoExW(cp, 0, &info)) continue;
		const bool skipped = IsSkipped(cp) || info.MaxCharSize > 2;
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

		// サロゲートの扱い（対、上位だけ、下位だけ）
		const WCHAR pair[] = { 0xD83D, 0xDE00 };
		const WCHAR hi[] = { 0xD83D };
		const WCHAR lo[] = { 0xDE00 };
		WriteWcResult(w, cp, pair, 2);
		WriteWcResult(w, cp, hi, 1);
		WriteWcResult(w, cp, lo, 1);

		// 1 バイト入力
		WCHAR single[256][8] = {};
		int singleLen[256] = {};
		for (int b = 0; b < 256; ++b) {
			const char c = static_cast<char>(b);
			singleLen[b] = ::MultiByteToWideChar(cp, 0, &c, 1, single[b], 8);
			if (singleLen[b] < 0) singleLen[b] = 0;
			WriteMbEntry(w, cp, &c, 1, nullptr, 0, nullptr, 0);
		}
		// 2 バイト入力（先行バイトごと）
		if (info.MaxCharSize == 2) {
			for (int lead = 0; lead < 256; ++lead) {
				if (!::IsDBCSLeadByteEx(cp, static_cast<BYTE>(lead))) continue;
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
