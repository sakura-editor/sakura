/*! @file
	@brief Mac 版的代码页转换（MultiByteToWideChar、WideCharToMultiByte 等）

	- 单字节/双字节代码页：使用在 Windows 上导出的转换表（macos/tools/codepage_dump），
	  结果（含 best fit、替代字符、MB_ERR_INVALID_CHARS 的判定）与 Windows 相同。
	- UTF-8：按 Windows 的规则实现（不正确序列的处理已在 Windows 上核对，见 DecodeUtf8）。
	- UTF-7、GB18030、ISO-2022 系、HZ：用 iconv 实现。
	- CP_ACP、CP_OEMCP、CP_THREAD_ACP 都是 65001（与 Windows 的 UTF-8 全球语言支持设置相同）。
*/
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <iconv.h>

// codepages.bin 的内容（由 codepage_data.cpp 用 .incbin 嵌入）
extern "C" const unsigned char g_sakuraCodePagesData[];
extern "C" const unsigned char g_sakuraCodePagesEnd[];
static const std::size_t g_sakuraCodePagesSize = static_cast<std::size_t>(g_sakuraCodePagesEnd - g_sakuraCodePagesData);

namespace {

//! 1 字节或 2 字节输入的转换结果
struct MbEntry {
	uint8_t flags;      // bit0: 指定 MB_ERR_INVALID_CHARS 时也成功；bit1: 只消耗前导字节
	uint8_t count;      // 输出的 UTF-16 码元数（0〜3）
	uint16_t out[3];
};

//! 一个宽字符的转换结果
struct WcResult {
	uint8_t len;
	uint8_t bytes[4];
	bool usedDefault;
};

struct CodePageTable {
	UINT cp = 0;
	UINT maxCharSize = 1;
	BYTE defaultChar[2] = {};
	WCHAR unicodeDefaultChar = 0;
	BYTE leadByte[12] = {};
	std::u16string name;
	WcResult surrogatePair{}, loneHigh{}, loneLow{};
	std::array<MbEntry, 256> single{};
	std::array<int, 256> leadIndex{};           // 在 pairs 中的序号（-1: 不是前导字节）
	std::vector<std::array<MbEntry, 256>> pairs;
	std::vector<uint64_t> wcBestFit;            // 65536 项。0: 没有对应，其他: (len << 32) | (b0 << 24) | (b1 << 16) | (b2 << 8) | b3
	std::vector<uint64_t> wcNoBestFit;
	std::map<uint32_t, WCHAR> gb4ToWide;        // GB18030 的 4 字节 BMP 字符（线性序号 → UTF-16），由 wcNoBestFit 反推

	int DefaultCharLen() const { return defaultChar[1] ? 2 : 1; }
};

class Reader {
public:
	Reader(const unsigned char* p, std::size_t n) : m_p(p), m_end(p + n) {}
	bool ok() const { return m_ok; }
	uint32_t u8() { if (m_p + 1 > m_end) { m_ok = false; return 0; } return *m_p++; }
	uint32_t u16() { const uint32_t a = u8(); return a | (u8() << 8); }
	uint32_t u32() { const uint32_t a = u16(); return a | (u16() << 16); }
	void skip(std::size_t n) { if (m_p + n > m_end) { m_ok = false; m_p = m_end; } else { m_p += n; } }
	const unsigned char* pos() const { return m_p; }
private:
	const unsigned char* m_p;
	const unsigned char* m_end;
	bool m_ok = true;
};

void ReadWcResult(Reader& r, WcResult& w)
{
	w.len = static_cast<uint8_t>(r.u8());
	for (auto& b : w.bytes) b = static_cast<uint8_t>(r.u8());
	w.usedDefault = r.u8() != 0;
}

void ReadMbEntry(Reader& r, MbEntry& e)
{
	e.flags = static_cast<uint8_t>(r.u8());
	e.count = static_cast<uint8_t>(r.u8());
	for (auto& c : e.out) c = static_cast<uint16_t>(r.u16());
}

void ReadWcTable(Reader& r, std::vector<uint64_t>& table)
{
	table.assign(0x10000, 0);
	const uint32_t n = r.u32();
	for (uint32_t i = 0; i < n && r.ok(); ++i) {
		const uint32_t u = r.u16();
		const uint64_t len = r.u8();
		uint64_t bytes = 0;
		for (int k = 0; k < 4; ++k) bytes = (bytes << 8) | r.u8();
		table[u] = (len << 32) | bytes;
	}
}

//! GB18030 的 4 字节序列 → 线性序号（81 30 81 30 为 0）
constexpr uint32_t Gb4Linear(uint32_t b1, uint32_t b2, uint32_t b3, uint32_t b4)
{
	return (((b1 - 0x81) * 10 + (b2 - 0x30)) * 126 + (b3 - 0x81)) * 10 + (b4 - 0x30);
}
constexpr uint32_t kGbSupplementaryBase = Gb4Linear(0x90, 0x30, 0x81, 0x30);   // U+10000

//! 全部代码页的转换表（第一次使用时读入）
class CodePageRegistry {
public:
	static CodePageRegistry& Instance()
	{
		static CodePageRegistry instance;
		return instance;
	}

	const CodePageTable* Find(UINT cp)
	{
		const auto it = m_offsets.find(cp);
		if (it == m_offsets.end()) return nullptr;
		std::lock_guard lock(m_mutex);
		auto& slot = m_tables[cp];
		if (!slot) slot = Load(it->second);
		return slot.get();
	}

	const std::map<UINT, std::size_t>& Offsets() const { return m_offsets; }

	// 只需要名称等信息时，不读整个表
	bool ReadHeader(UINT cp, CodePageTable& out) const
	{
		const auto it = m_offsets.find(cp);
		if (it == m_offsets.end()) return false;
		Reader r(g_sakuraCodePagesData + it->second, g_sakuraCodePagesSize - it->second);
		ReadHeaderFields(r, out);
		return r.ok();
	}

private:
	CodePageRegistry()
	{
		Reader r(g_sakuraCodePagesData, g_sakuraCodePagesSize);
		if (g_sakuraCodePagesSize < 12 || std::memcmp(g_sakuraCodePagesData, "SKCP", 4) != 0) return;
		r.skip(4);
		if (r.u32() != 2) return;
		const uint32_t count = r.u32();
		for (uint32_t i = 0; i < count && r.ok(); ++i) {
			const std::size_t offset = static_cast<std::size_t>(r.pos() - g_sakuraCodePagesData);
			CodePageTable t;
			ReadHeaderFields(r, t);
			SkipBody(r, t);
			if (r.ok()) m_offsets.emplace(t.cp, offset);
		}
	}

	static void ReadHeaderFields(Reader& r, CodePageTable& t)
	{
		t.cp = r.u32();
		t.maxCharSize = r.u8();
		t.defaultChar[0] = static_cast<BYTE>(r.u8());
		t.defaultChar[1] = static_cast<BYTE>(r.u8());
		t.unicodeDefaultChar = static_cast<WCHAR>(r.u16());
		for (auto& b : t.leadByte) b = static_cast<BYTE>(r.u8());
		const uint32_t nameLen = r.u16();
		t.name.clear();
		for (uint32_t i = 0; i < nameLen; ++i) t.name.push_back(static_cast<char16_t>(r.u16()));
	}

	static int CountLeadBytes(const CodePageTable& t, const std::array<MbEntry, 256>&)
	{
		int n = 0;
		for (int b = 0; b < 256; ++b) if (IsLead(t, static_cast<BYTE>(b))) ++n;
		return n;
	}

	static bool IsLead(const CodePageTable& t, BYTE b)
	{
		if (t.cp == 54936) return 0x81 <= b && b <= 0xFE;   // GB18030（导出工具也按这个范围导出）
		if (t.maxCharSize != 2) return false;
		for (int i = 0; i + 1 < 12 && (t.leadByte[i] || t.leadByte[i + 1]); i += 2) {
			if (t.leadByte[i] <= b && b <= t.leadByte[i + 1]) return true;
		}
		return false;
	}

	static void SkipBody(Reader& r, const CodePageTable& t)
	{
		r.skip(3 * 6);                                  // 代理对的 3 项
		r.skip(256 * 8);                                // 1 字节输入
		r.skip(static_cast<std::size_t>(CountLeadBytes(t, {})) * 256 * 8);
		for (int k = 0; k < 2 && r.ok(); ++k) {
			const uint32_t n = r.u32();
			r.skip(static_cast<std::size_t>(n) * 7);
		}
	}

	std::unique_ptr<CodePageTable> Load(std::size_t offset) const
	{
		auto t = std::make_unique<CodePageTable>();
		Reader r(g_sakuraCodePagesData + offset, g_sakuraCodePagesSize - offset);
		ReadHeaderFields(r, *t);
		ReadWcResult(r, t->surrogatePair);
		ReadWcResult(r, t->loneHigh);
		ReadWcResult(r, t->loneLow);
		for (auto& e : t->single) ReadMbEntry(r, e);
		t->leadIndex.fill(-1);
		for (int b = 0; b < 256; ++b) {
			if (!IsLead(*t, static_cast<BYTE>(b))) continue;
			t->leadIndex[b] = static_cast<int>(t->pairs.size());
			auto& row = t->pairs.emplace_back();
			for (auto& e : row) ReadMbEntry(r, e);
		}
		ReadWcTable(r, t->wcBestFit);
		ReadWcTable(r, t->wcNoBestFit);
		if (t->cp == 54936) {
			for (uint32_t u = 0; u < 0x10000; ++u) {
				const uint64_t v = t->wcNoBestFit[u];
				if ((v >> 32) != 4) continue;
				t->gb4ToWide.emplace(Gb4Linear((v >> 24) & 0xFF, (v >> 16) & 0xFF, (v >> 8) & 0xFF, v & 0xFF), static_cast<WCHAR>(u));
			}
		}
		if (!r.ok()) return nullptr;
		return t;
	}

	std::map<UINT, std::size_t> m_offsets;
	std::map<UINT, std::unique_ptr<CodePageTable>> m_tables;
	std::mutex m_mutex;
};

// ---------------------------------------------------------------------------
// 用 iconv 实现的代码页
// ---------------------------------------------------------------------------
struct IconvCodePage {
	UINT cp;
	const char* iconvName;
	const char16_t* name;
	UINT maxCharSize;
};

constexpr IconvCodePage kIconvCodePages[] = {
	{ 50220, "ISO-2022-JP",   u"50220 (ISO-2022 Japanese with no halfwidth Katakana)", 5 },
	{ 50221, "ISO-2022-JP-1", u"50221 (ISO-2022 Japanese with halfwidth Katakana)", 5 },
	{ 50222, "ISO-2022-JP-1", u"50222 (ISO-2022 Japanese JIS X 0201-1989)", 5 },
	{ 50225, "ISO-2022-KR",   u"50225 (ISO-2022 Korean)", 5 },
	{ 50227, "ISO-2022-CN",   u"50227 (ISO-2022 Simplified Chinese)", 5 },
	{ 50229, "ISO-2022-CN-EXT", u"50229 (ISO-2022 Traditional Chinese)", 5 },
	{ 52936, "HZ-GB-2312",    u"52936 (HZ-GB2312 Simplified Chinese)", 5 },
};

const IconvCodePage* FindIconv(UINT cp)
{
	for (const auto& c : kIconvCodePages) {
		if (c.cp == cp) return &c;
	}
	return nullptr;
}

bool IconvAvailable(const IconvCodePage& c)
{
	iconv_t h = ::iconv_open("UTF-16LE", c.iconvName);
	if (h == reinterpret_cast<iconv_t>(-1)) return false;
	::iconv_close(h);
	return true;
}

//! 用 iconv 转换。有不正确的输入或无法转换的字符时返回 false
bool IconvConvert(const char* to, const char* from, const char* src, std::size_t srcLen, std::string& out)
{
	iconv_t h = ::iconv_open(to, from);
	if (h == reinterpret_cast<iconv_t>(-1)) return false;
	out.clear();
	char buf[1024];
	auto* in = const_cast<char*>(src);
	std::size_t inLeft = srcLen;
	bool ok = true;
	while (inLeft > 0) {
		char* o = buf;
		std::size_t oLeft = sizeof(buf);
		const std::size_t r = ::iconv(h, &in, &inLeft, &o, &oLeft);
		out.append(buf, static_cast<std::size_t>(o - buf));
		if (r == static_cast<std::size_t>(-1)) {
			if (errno == E2BIG) continue;
			ok = false;
			break;
		}
	}
	if (ok) {
		char* o = buf;
		std::size_t oLeft = sizeof(buf);
		::iconv(h, nullptr, nullptr, &o, &oLeft);   // 恢复移位状态
		out.append(buf, static_cast<std::size_t>(o - buf));
	}
	::iconv_close(h);
	return ok;
}

// ---------------------------------------------------------------------------
// 共通
// ---------------------------------------------------------------------------
UINT ResolveCodePage(UINT cp)
{
	switch (cp) {
	case CP_ACP: case CP_OEMCP: case CP_THREAD_ACP:
		return CP_UTF8;
	case CP_MACCP:
		return 10000;
	default:
		return cp;
	}
}

//! 写入输出缓冲区（容量为 0 时只计数）
template<typename T>
class Output {
public:
	Output(T* dst, int cap) : m_dst(dst), m_cap(cap) {}
	bool push(T v)
	{
		if (m_cap != 0) {
			if (m_n >= m_cap) { m_overflow = true; return false; }
			m_dst[m_n] = v;
		}
		++m_n;
		return true;
	}
	int count() const { return m_n; }
	bool overflow() const { return m_overflow; }
private:
	T* m_dst;
	int m_cap;
	int m_n = 0;
	bool m_overflow = false;
};

int Fail(DWORD err)
{
	SetLastError(err);
	return 0;
}

// ---------------------------------------------------------------------------
// UTF-8
// ---------------------------------------------------------------------------
//! 读一个 UTF-8 字符。不正确时把消耗的字节数放进 len 并返回 false。
//! 与 Windows 的 MultiByteToWideChar 相同：第 2 字节是后续字节（80〜BF）但超出该前导字节允许的范围
//! （过长编码、代理区、超过 U+10FFFF）时，前 2 字节合起来算一个不正确的字符；其余按“最大子部分”处理。
bool DecodeUtf8(const unsigned char* p, int n, char32_t& cp, int& len)
{
	const unsigned char c = p[0];
	if (c < 0x80) { cp = c; len = 1; return true; }
	int need;
	char32_t v;
	unsigned char lo = 0x80, hi = 0xBF;
	if (0xC2 <= c && c <= 0xDF) { need = 1; v = c & 0x1F; }
	else if (0xE0 <= c && c <= 0xEF) {
		need = 2; v = c & 0x0F;
		if (c == 0xE0) lo = 0xA0;
		if (c == 0xED) hi = 0x9F;           // 代理区是不正确的
	}
	else if (0xF0 <= c && c <= 0xF4) {
		need = 3; v = c & 0x07;
		if (c == 0xF0) lo = 0x90;
		if (c == 0xF4) hi = 0x8F;
	}
	else { len = 1; return false; }
	for (int i = 1; i <= need; ++i) {
		if (i >= n) { len = i; return false; }
		const unsigned char t = p[i];
		if (t < 0x80 || 0xBF < t) { len = i; return false; }
		if (i == 1 && (t < lo || hi < t)) { len = 2; return false; }
		v = (v << 6) | (t & 0x3F);
	}
	cp = v;
	len = need + 1;
	return true;
}

int Utf8ToWide(DWORD flags, const unsigned char* src, int n, WCHAR* dst, int cap)
{
	if (flags & ~static_cast<DWORD>(MB_ERR_INVALID_CHARS)) return Fail(ERROR_INVALID_FLAGS);
	Output<WCHAR> out(dst, cap);
	for (int i = 0; i < n;) {
		char32_t c;
		int len;
		if (!DecodeUtf8(src + i, n - i, c, len)) {
			if (flags & MB_ERR_INVALID_CHARS) return Fail(ERROR_NO_UNICODE_TRANSLATION);
			if (!out.push(0xFFFD)) break;
			i += len;
			continue;
		}
		if (c >= 0x10000) {
			c -= 0x10000;
			if (!out.push(static_cast<WCHAR>(0xD800 + (c >> 10)))) break;
			if (!out.push(static_cast<WCHAR>(0xDC00 + (c & 0x3FF)))) break;
		} else {
			if (!out.push(static_cast<WCHAR>(c))) break;
		}
		i += len;
	}
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	return out.count();
}

int WideToUtf8(DWORD flags, const WCHAR* src, int n, char* dst, int cap, LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar)
{
	if (flags & ~static_cast<DWORD>(WC_ERR_INVALID_CHARS)) return Fail(ERROR_INVALID_FLAGS);
	// 指定了 lpUsedDefaultChar 也不报错、不使用（已在 Windows 上确认）
	(void)lpUsedDefaultChar;
	if (lpDefaultChar) return Fail(ERROR_INVALID_PARAMETER);
	Output<char> out(dst, cap);
	auto put = [&](char32_t c) {
		if (c < 0x80) return out.push(static_cast<char>(c));
		if (c < 0x800) return out.push(static_cast<char>(0xC0 | (c >> 6))) && out.push(static_cast<char>(0x80 | (c & 0x3F)));
		if (c < 0x10000) return out.push(static_cast<char>(0xE0 | (c >> 12))) && out.push(static_cast<char>(0x80 | ((c >> 6) & 0x3F))) && out.push(static_cast<char>(0x80 | (c & 0x3F)));
		return out.push(static_cast<char>(0xF0 | (c >> 18))) && out.push(static_cast<char>(0x80 | ((c >> 12) & 0x3F)))
			&& out.push(static_cast<char>(0x80 | ((c >> 6) & 0x3F))) && out.push(static_cast<char>(0x80 | (c & 0x3F)));
	};
	for (int i = 0; i < n; ++i) {
		const char16_t u = src[i];
		char32_t c = u;
		if (0xD800 <= u && u <= 0xDBFF && i + 1 < n && 0xDC00 <= src[i + 1] && src[i + 1] <= 0xDFFF) {
			c = 0x10000 + ((u - 0xD800) << 10) + (src[i + 1] - 0xDC00);
			++i;
		} else if (0xD800 <= u && u <= 0xDFFF) {
			if (flags & WC_ERR_INVALID_CHARS) return Fail(ERROR_NO_UNICODE_TRANSLATION);
			c = 0xFFFD;
		}
		if (!put(c)) break;
	}
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	return out.count();
}

// ---------------------------------------------------------------------------
// UTF-7（行为按 Windows 上的探测结果实现，见 macos/docs/phase1-charset.md）
// ---------------------------------------------------------------------------
constexpr char kBase64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int Base64Value(unsigned char c)
{
	if ('A' <= c && c <= 'Z') return c - 'A';
	if ('a' <= c && c <= 'z') return c - 'a' + 26;
	if ('0' <= c && c <= '9') return c - '0' + 52;
	if (c == '+') return 62;
	if (c == '/') return 63;
	return -1;
}

//! Windows 不经 base64 直接输出的字符
bool IsUtf7Direct(char16_t c)
{
	if (('A' <= c && c <= 'Z') || ('a' <= c && c <= 'z') || ('0' <= c && c <= '9')) return true;
	switch (c) {
	case 0x00: case u'\t': case u'\n': case u'\r': case u' ':
	case u'\'': case u'(': case u')': case u',': case u'-': case u'.': case u'/': case u':': case u'?':
		return true;
	default:
		return false;
	}
}

int Utf7ToWide(DWORD flags, const unsigned char* src, int n, WCHAR* dst, int cap)
{
	if (flags != 0) return Fail(ERROR_INVALID_FLAGS);   // MB_ERR_INVALID_CHARS 也是 1004（与 Windows 相同）
	Output<WCHAR> out(dst, cap);
	for (int i = 0; i < n;) {
		const unsigned char c = src[i];
		if (c != '+') {
			if (!out.push(c)) break;          // 0x80 以上的字节也按 Latin-1 原样输出
			++i;
			continue;
		}
		++i;
		if (i < n && src[i] == '-') {         // "+-" → '+'
			if (!out.push(u'+')) break;
			++i;
			continue;
		}
		uint32_t bits = 0;
		int nbits = 0;
		bool full = false;
		for (; i < n; ++i) {
			const int v = Base64Value(src[i]);
			if (v < 0) break;
			bits = (bits << 6) | static_cast<uint32_t>(v);
			nbits += 6;
			if (nbits >= 16) {
				nbits -= 16;
				if (!out.push(static_cast<WCHAR>((bits >> nbits) & 0xFFFF))) { full = true; break; }
			}
			bits &= (1u << nbits) - 1;
		}
		if (full) break;
		if (i < n && src[i] == '-') ++i;      // 结束标志 '-' 被吸收，其他字符保留
	}
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	return out.count();
}

int WideToUtf7(DWORD flags, const WCHAR* src, int n, char* dst, int cap, LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar)
{
	if (flags != 0) return Fail(ERROR_INVALID_FLAGS);
	(void)lpUsedDefaultChar;
	if (lpDefaultChar) return Fail(ERROR_INVALID_PARAMETER);
	Output<char> out(dst, cap);
	for (int i = 0; i < n;) {
		const char16_t c = src[i];
		if (c == u'+') {
			if (!out.push('+') || !out.push('-')) break;
			++i;
			continue;
		}
		if (IsUtf7Direct(c)) {
			if (!out.push(static_cast<char>(c))) break;
			++i;
			continue;
		}
		// 连续的非直接字符合成一个 base64 块（代理也按 16 位原样编码），块总是以 '-' 结束
		if (!out.push('+')) break;
		uint32_t bits = 0;
		int nbits = 0;
		bool ok = true;
		for (; i < n && src[i] != u'+' && !IsUtf7Direct(src[i]); ++i) {
			bits = (bits << 16) | src[i];
			nbits += 16;
			while (nbits >= 6 && ok) {
				nbits -= 6;
				ok = out.push(kBase64Chars[(bits >> nbits) & 0x3F]);
			}
			bits &= (1u << nbits) - 1;
			if (!ok) break;
		}
		if (ok && nbits > 0) ok = out.push(kBase64Chars[(bits << (6 - nbits)) & 0x3F]);
		if (!ok || !out.push('-')) break;
	}
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	return out.count();
}

// ---------------------------------------------------------------------------
// 変換表
// ---------------------------------------------------------------------------
int TableToWide(const CodePageTable& t, DWORD flags, const unsigned char* src, int n, WCHAR* dst, int cap)
{
	if (flags & ~static_cast<DWORD>(MB_PRECOMPOSED | MB_ERR_INVALID_CHARS)) return Fail(ERROR_INVALID_FLAGS);
	const bool strict = (flags & MB_ERR_INVALID_CHARS) != 0;
	Output<WCHAR> out(dst, cap);
	for (int i = 0; i < n && !out.overflow();) {
		const unsigned char b = src[i];
		// GB18030 的 4 字节字符（格式正确但没有对应的字符变成 '?'，与 Windows 相同，不算错误）
		if (t.cp == 54936 && 0x81 <= b && b <= 0xFE && i + 3 < n
			&& 0x30 <= src[i + 1] && src[i + 1] <= 0x39 && 0x81 <= src[i + 2] && src[i + 2] <= 0xFE
			&& 0x30 <= src[i + 3] && src[i + 3] <= 0x39) {
			const uint32_t linear = Gb4Linear(b, src[i + 1], src[i + 2], src[i + 3]);
			if (linear >= kGbSupplementaryBase && linear - kGbSupplementaryBase < 0x100000) {
				const uint32_t c = linear - kGbSupplementaryBase;
				if (!out.push(static_cast<WCHAR>(0xD800 + (c >> 10)))) break;
				if (!out.push(static_cast<WCHAR>(0xDC00 + (c & 0x3FF)))) break;
			} else if (const auto it = t.gb4ToWide.find(linear); it != t.gb4ToWide.end()) {
				if (!out.push(it->second)) break;
			} else {
				if (!out.push(u'?')) break;
			}
			i += 4;
			continue;
		}
		const MbEntry* e = &t.single[b];
		int consumed = 1;
		if (t.leadIndex[b] >= 0 && i + 1 < n) {
			const MbEntry& pe = t.pairs[static_cast<std::size_t>(t.leadIndex[b])][src[i + 1]];
			if (!(pe.flags & 2)) { e = &pe; consumed = 2; }
			else if (strict && !(pe.flags & 1)) return Fail(ERROR_NO_UNICODE_TRANSLATION);
		}
		if (strict && !(e->flags & 1)) return Fail(ERROR_NO_UNICODE_TRANSLATION);
		for (int k = 0; k < e->count; ++k) {
			if (!out.push(e->out[k])) break;
		}
		i += consumed;
	}
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	return out.count();
}

int TableToMulti(const CodePageTable& t, DWORD flags, const WCHAR* src, int n, char* dst, int cap, LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar)
{
	// WC_SEPCHARS、WC_DISCARDNS、WC_DEFAULTCHAR 只在与 WC_COMPOSITECHECK 一起使用时才有意义。
	// 转换表只处理已合成的字符，所以接受这些标志但不做处理
	if (flags & ~static_cast<DWORD>(WC_NO_BEST_FIT_CHARS | WC_COMPOSITECHECK | WC_DEFAULTCHAR | WC_SEPCHARS | WC_DISCARDNS)) return Fail(ERROR_INVALID_FLAGS);
	const auto& table = (flags & WC_NO_BEST_FIT_CHARS) ? t.wcNoBestFit : t.wcBestFit;
	// 替代字符
	char def[2] = { static_cast<char>(t.defaultChar[0]), static_cast<char>(t.defaultChar[1]) };
	int defLen = t.DefaultCharLen();
	if (lpDefaultChar) {
		def[0] = lpDefaultChar[0];
		def[1] = (t.maxCharSize == 2 && lpDefaultChar[0] && IsDBCSLeadByteEx(t.cp, static_cast<BYTE>(lpDefaultChar[0]))) ? lpDefaultChar[1] : 0;
		defLen = def[1] ? 2 : 1;
	}
	Output<char> out(dst, cap);
	bool used = false;
	auto putDefault = [&] {
		used = true;
		for (int k = 0; k < defLen; ++k) { if (!out.push(def[k])) return false; }
		return true;
	};
	auto putRecorded = [&](const WcResult& r) {
		if (r.usedDefault) {
			// 按记录的替代字符个数，输出指定的替代字符
			const int times = std::max(1, r.len / t.DefaultCharLen());
			for (int k = 0; k < times; ++k) { if (!putDefault()) return false; }
			return true;
		}
		for (int k = 0; k < r.len; ++k) { if (!out.push(static_cast<char>(r.bytes[k]))) return false; }
		return true;
	};
	for (int i = 0; i < n && !out.overflow(); ++i) {
		const char16_t u = src[i];
		if (0xD800 <= u && u <= 0xDBFF && i + 1 < n && 0xDC00 <= src[i + 1] && src[i + 1] <= 0xDFFF) {
			if (t.cp == 54936) {
				// GB18030：补充平面的字符按公式换成 4 字节
				uint32_t linear = kGbSupplementaryBase + ((u - 0xD800u) << 10) + (src[i + 1] - 0xDC00u);
				char bytes[4];
				bytes[3] = static_cast<char>(0x30 + linear % 10); linear /= 10;
				bytes[2] = static_cast<char>(0x81 + linear % 126); linear /= 126;
				bytes[1] = static_cast<char>(0x30 + linear % 10); linear /= 10;
				bytes[0] = static_cast<char>(0x81 + linear);
				for (char c : bytes) { if (!out.push(c)) break; }
			} else {
				putRecorded(t.surrogatePair);
			}
			++i;
			continue;
		}
		if (0xD800 <= u && u <= 0xDBFF) { putRecorded(t.loneHigh); continue; }
		if (0xDC00 <= u && u <= 0xDFFF) { putRecorded(t.loneLow); continue; }
		const uint64_t v = table[u];
		if (v == 0) { putDefault(); continue; }
		const int len = static_cast<int>(v >> 32);
		for (int k = 0; k < len; ++k) {
			if (!out.push(static_cast<char>((v >> (24 - 8 * k)) & 0xFF))) break;
		}
	}
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	if (lpUsedDefaultChar) *lpUsedDefaultChar = used ? TRUE : FALSE;
	return out.count();
}

// ---------------------------------------------------------------------------
// iconv
// ---------------------------------------------------------------------------
int IconvToWide(const IconvCodePage& c, DWORD flags, const char* src, int n, WCHAR* dst, int cap)
{
	if (flags & ~static_cast<DWORD>(MB_ERR_INVALID_CHARS)) return Fail(ERROR_INVALID_FLAGS);
	std::string u16;
	if (!IconvConvert("UTF-16LE", c.iconvName, src, static_cast<std::size_t>(n), u16)) {
		if (flags & MB_ERR_INVALID_CHARS) return Fail(ERROR_NO_UNICODE_TRANSLATION);
		// 输出已转换的部分，再加一个替代字符
		u16.append("\xFD\xFF", 2);
	}
	Output<WCHAR> out(dst, cap);
	for (std::size_t i = 0; i + 1 < u16.size(); i += 2) {
		const auto w = static_cast<WCHAR>(static_cast<unsigned char>(u16[i]) | (static_cast<unsigned char>(u16[i + 1]) << 8));
		if (!out.push(w)) break;
	}
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	return out.count();
}

int IconvToMulti(const IconvCodePage& c, DWORD flags, const WCHAR* src, int n, char* dst, int cap, LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar)
{
	if (flags != 0) return Fail(ERROR_INVALID_FLAGS);
	if (lpDefaultChar || lpUsedDefaultChar) return Fail(ERROR_INVALID_PARAMETER);
	std::string mb;
	// 无法转换的字符逐个换成 '?'
	std::string all;
	for (int i = 0; i < n; ++i) {
		int len = (0xD800 <= src[i] && src[i] <= 0xDBFF && i + 1 < n && 0xDC00 <= src[i + 1] && src[i + 1] <= 0xDFFF) ? 2 : 1;
		std::string one(reinterpret_cast<const char*>(src + i), static_cast<std::size_t>(len) * 2);
		i += len - 1;
		all += one;
	}
	if (!IconvConvert(c.iconvName, "UTF-16LE", all.data(), all.size(), mb)) {
		mb.clear();
		std::string piece;
		for (std::size_t i = 0; i < all.size(); ) {
			const auto u = static_cast<char16_t>(static_cast<unsigned char>(all[i]) | (static_cast<unsigned char>(all[i + 1]) << 8));
			const std::size_t len = (0xD800 <= u && u <= 0xDBFF && i + 3 < all.size()) ? 4 : 2;
			if (IconvConvert(c.iconvName, "UTF-16LE", all.data() + i, len, piece)) mb += piece;
			else mb += '?';
			i += len;
		}
	}
	Output<char> out(dst, cap);
	for (char ch : mb) { if (!out.push(ch)) break; }
	if (out.overflow()) return Fail(ERROR_INSUFFICIENT_BUFFER);
	return out.count();
}

} // namespace

// ---------------------------------------------------------------------------
// 公開関数
// ---------------------------------------------------------------------------
UINT GetACP() { return CP_UTF8; }
UINT GetOEMCP() { return CP_UTF8; }

BOOL IsValidCodePage(UINT codePage)
{
	if (codePage == CP_UTF8 || codePage == CP_UTF7) return TRUE;
	if (CodePageRegistry::Instance().Offsets().contains(codePage)) return TRUE;
	if (const auto* c = FindIconv(codePage)) return IconvAvailable(*c) ? TRUE : FALSE;
	return FALSE;
}

BOOL GetCPInfoExW(UINT codePage, DWORD dwFlags, LPCPINFOEXW info)
{
	if (!info || dwFlags != 0) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
	const UINT cp = ResolveCodePage(codePage);
	*info = CPINFOEXW{};
	info->CodePage = cp;
	auto setName = [&](const std::u16string& name) {
		const std::size_t n = std::min<std::size_t>(name.size(), MAX_PATH - 1);
		std::copy_n(name.data(), n, info->CodePageName);
		info->CodePageName[n] = 0;
	};
	if (cp == CP_UTF8) {
		info->MaxCharSize = 4;
		info->DefaultChar[0] = '?';
		info->UnicodeDefaultChar = 0xFFFD;
		setName(u"65001 (UTF-8)");
		return TRUE;
	}
	if (cp == CP_UTF7) {
		info->MaxCharSize = 5;
		info->DefaultChar[0] = '?';
		info->UnicodeDefaultChar = 0xFFFD;
		setName(u"65000 (UTF-7)");
		return TRUE;
	}
	CodePageTable t;
	if (CodePageRegistry::Instance().ReadHeader(cp, t)) {
		info->MaxCharSize = t.maxCharSize;
		std::copy_n(t.defaultChar, 2, info->DefaultChar);
		std::copy_n(t.leadByte, 12, info->LeadByte);
		info->UnicodeDefaultChar = t.unicodeDefaultChar;
		setName(t.name);
		return TRUE;
	}
	if (const auto* c = FindIconv(cp); c && IconvAvailable(*c)) {
		info->MaxCharSize = c->maxCharSize;
		info->DefaultChar[0] = '?';
		info->UnicodeDefaultChar = 0xFFFD;
		setName(c->name);
		return TRUE;
	}
	SetLastError(ERROR_INVALID_PARAMETER);
	return FALSE;
}

BOOL GetCPInfo(UINT codePage, LPCPINFO info)
{
	if (!info) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
	CPINFOEXW ex;
	if (!GetCPInfoExW(codePage, 0, &ex)) return FALSE;
	info->MaxCharSize = ex.MaxCharSize;
	std::copy_n(ex.DefaultChar, 2, info->DefaultChar);
	std::copy_n(ex.LeadByte, 12, info->LeadByte);
	return TRUE;
}

BOOL IsDBCSLeadByteEx(UINT codePage, BYTE testChar)
{
	CPINFO info;
	if (!GetCPInfo(codePage, &info) || info.MaxCharSize != 2) return FALSE;
	for (int i = 0; i + 1 < MAX_LEADBYTES && (info.LeadByte[i] || info.LeadByte[i + 1]); i += 2) {
		if (info.LeadByte[i] <= testChar && testChar <= info.LeadByte[i + 1]) return TRUE;
	}
	return FALSE;
}

BOOL IsDBCSLeadByte(BYTE testChar)
{
	return IsDBCSLeadByteEx(CP_ACP, testChar);
}

BOOL EnumSystemCodePagesW(CODEPAGE_ENUMPROCW proc, DWORD dwFlags)
{
	if (!proc || (dwFlags != CP_INSTALLED && dwFlags != CP_SUPPORTED)) { SetLastError(ERROR_INVALID_PARAMETER); return FALSE; }
	std::vector<UINT> cps;
	for (const auto& [cp, offset] : CodePageRegistry::Instance().Offsets()) cps.push_back(cp);
	for (const auto& c : kIconvCodePages) { if (IconvAvailable(c)) cps.push_back(c.cp); }
	cps.push_back(CP_UTF7);
	cps.push_back(CP_UTF8);
	std::sort(cps.begin(), cps.end());
	for (UINT cp : cps) {
		auto s = sakura::u16::from_utf8<char16_t>(std::to_string(cp));
		if (!proc(s.data())) break;
	}
	return TRUE;
}

int MultiByteToWideChar(UINT codePage, DWORD dwFlags, LPCSTR src, int cbSrc, WCHAR* dst, int cchDst)
{
	if (!src || cbSrc == 0 || cchDst < 0 || (cchDst > 0 && !dst)) return Fail(ERROR_INVALID_PARAMETER);
	const int n = (cbSrc < 0) ? static_cast<int>(std::strlen(src)) + 1 : cbSrc;
	const UINT cp = ResolveCodePage(codePage);
	const auto* u = reinterpret_cast<const unsigned char*>(src);
	if (cp == CP_UTF8) return Utf8ToWide(dwFlags, u, n, dst, cchDst);
	if (cp == CP_UTF7) return Utf7ToWide(dwFlags, u, n, dst, cchDst);
	if (const auto* t = CodePageRegistry::Instance().Find(cp)) return TableToWide(*t, dwFlags, u, n, dst, cchDst);
	if (const auto* c = FindIconv(cp); c && IconvAvailable(*c)) return IconvToWide(*c, dwFlags, src, n, dst, cchDst);
	return Fail(ERROR_INVALID_PARAMETER);
}

int WideCharToMultiByte(UINT codePage, DWORD dwFlags, const WCHAR* src, int cchSrc, LPSTR dst, int cbDst, LPCSTR lpDefaultChar, BOOL* lpUsedDefaultChar)
{
	if (!src || cchSrc == 0 || cbDst < 0 || (cbDst > 0 && !dst)) return Fail(ERROR_INVALID_PARAMETER);
	int n = cchSrc;
	if (n < 0) { n = 0; while (src[n]) ++n; ++n; }
	const UINT cp = ResolveCodePage(codePage);
	if (cp == CP_UTF8) return WideToUtf8(dwFlags, src, n, dst, cbDst, lpDefaultChar, lpUsedDefaultChar);
	if (cp == CP_UTF7) return WideToUtf7(dwFlags, src, n, dst, cbDst, lpDefaultChar, lpUsedDefaultChar);
	if (const auto* t = CodePageRegistry::Instance().Find(cp)) return TableToMulti(*t, dwFlags, src, n, dst, cbDst, lpDefaultChar, lpUsedDefaultChar);
	if (const auto* c = FindIconv(cp); c && IconvAvailable(*c)) return IconvToMulti(*c, dwFlags, src, n, dst, cbDst, lpDefaultChar, lpUsedDefaultChar);
	return Fail(ERROR_INVALID_PARAMETER);
}
