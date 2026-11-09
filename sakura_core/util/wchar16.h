/*! @file
	@brief UTF-16 字符类型与字符串工具（Mac 版移植 阶段 1 第 14 步）

	方案：
	- WCHAR 在所有平台上都是 2 字节的 UTF-16 码元。
	  - Windows：WCHAR = wchar_t（Windows SDK 原有定义，原版代码完全不变）。
	  - 其他平台：WCHAR = char16_t（由 sakura_platform.h 定义）。
	- 宽字符字面量统一写成 LTEXT("...") / LCHAR('x')（util/std_macro.h 原有的宏），
	  Windows 上展开为 L"..."，其他平台展开为 u"..."。
	- 字符串函数：非 Windows 平台上为 char16_t 提供与 MSVC 同名、同语义的重载
	  （wcslen、wcscmp、_wcsicmp、_swprintf 等），因此调用这些函数的代码不需要改写。
	  实现都在 sakura::u16 命名空间里，以模板形式提供，两个平台都能使用和测试。
	- std::wstring / std::wstring_view 改用 sakura::wstring / sakura::wstring_view。

	Copyright (C) 2026, Sakura Editor Organization
	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_WCHAR16_H_
#define SAKURA_WCHAR16_H_
#pragma once

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include <cwctype>
#include <string>
#include <string_view>
#include <type_traits>

#if defined(_WIN32)
#  define SAKURA_WCHAR_IS_CHAR16 0
#else
#  define SAKURA_WCHAR_IS_CHAR16 1
#endif

static_assert(sizeof(WCHAR) == 2, "WCHAR must be a 16-bit UTF-16 code unit.");
#if SAKURA_WCHAR_IS_CHAR16
static_assert(std::is_same_v<WCHAR, char16_t>, "WCHAR must be char16_t on non-Windows platforms.");
#endif

namespace sakura {

//! WCHAR の文字列（std::wstring の代わり）
using wstring = std::basic_string<WCHAR>;
//! WCHAR の文字列ビュー（std::wstring_view の代わり）
using wstring_view = std::basic_string_view<WCHAR>;

namespace u16 {

template <typename C>
concept CodeUnit16 = (sizeof(C) == 2) && (std::is_same_v<C, char16_t> || std::is_same_v<C, wchar_t>);

// ---------------------------------------------------------------------------
// 文字の判定と大小文字変換
// ---------------------------------------------------------------------------
constexpr bool is_high_surrogate(char32_t c) noexcept { return 0xD800 <= c && c <= 0xDBFF; }
constexpr bool is_low_surrogate(char32_t c) noexcept { return 0xDC00 <= c && c <= 0xDFFF; }

//! 1 文字を小文字にする（サロゲートはそのまま）
template <CodeUnit16 C>
inline C to_lower(C c) noexcept {
	const auto u = static_cast<char32_t>(static_cast<std::uint16_t>(c));
	if (u < 0x80) return (u >= 'A' && u <= 'Z') ? static_cast<C>(u + ('a' - 'A')) : c;
	if (is_high_surrogate(u) || is_low_surrogate(u)) return c;
	return static_cast<C>(std::towlower(static_cast<std::wint_t>(u)));
}
//! 1 文字を大文字にする（サロゲートはそのまま）
template <CodeUnit16 C>
inline C to_upper(C c) noexcept {
	const auto u = static_cast<char32_t>(static_cast<std::uint16_t>(c));
	if (u < 0x80) return (u >= 'a' && u <= 'z') ? static_cast<C>(u - ('a' - 'A')) : c;
	if (is_high_surrogate(u) || is_low_surrogate(u)) return c;
	return static_cast<C>(std::towupper(static_cast<std::wint_t>(u)));
}
template <CodeUnit16 C> inline bool is_space(C c) noexcept { return std::iswspace(static_cast<std::wint_t>(static_cast<std::uint16_t>(c))) != 0; }
template <CodeUnit16 C> inline bool is_alpha(C c) noexcept { return std::iswalpha(static_cast<std::wint_t>(static_cast<std::uint16_t>(c))) != 0; }
template <CodeUnit16 C> inline bool is_digit(C c) noexcept { return c >= C('0') && c <= C('9'); }

// ---------------------------------------------------------------------------
// C 文字列関数（wcs* / wmem* 相当）
// ---------------------------------------------------------------------------
template <CodeUnit16 C> constexpr size_t strlen(const C* s) noexcept { size_t n = 0; while (s[n]) ++n; return n; }
template <CodeUnit16 C> constexpr size_t strnlen(const C* s, size_t max) noexcept { size_t n = 0; while (n < max && s[n]) ++n; return n; }

template <CodeUnit16 C> constexpr int strcmp(const C* a, const C* b) noexcept {
	for (;; ++a, ++b) {
		const auto x = static_cast<std::uint16_t>(*a), y = static_cast<std::uint16_t>(*b);
		if (x != y) return x < y ? -1 : 1;
		if (!x) return 0;
	}
}
template <CodeUnit16 C> constexpr int strncmp(const C* a, const C* b, size_t n) noexcept {
	for (; n; --n, ++a, ++b) {
		const auto x = static_cast<std::uint16_t>(*a), y = static_cast<std::uint16_t>(*b);
		if (x != y) return x < y ? -1 : 1;
		if (!x) return 0;
	}
	return 0;
}
template <CodeUnit16 C> inline int stricmp(const C* a, const C* b) noexcept {
	for (;; ++a, ++b) {
		const auto x = static_cast<std::uint16_t>(to_lower(*a)), y = static_cast<std::uint16_t>(to_lower(*b));
		if (x != y) return x < y ? -1 : 1;
		if (!x) return 0;
	}
}
template <CodeUnit16 C> inline int strnicmp(const C* a, const C* b, size_t n) noexcept {
	for (; n; --n, ++a, ++b) {
		const auto x = static_cast<std::uint16_t>(to_lower(*a)), y = static_cast<std::uint16_t>(to_lower(*b));
		if (x != y) return x < y ? -1 : 1;
		if (!x) return 0;
	}
	return 0;
}
template <CodeUnit16 C> constexpr const C* strchr(const C* s, C c) noexcept {
	for (;; ++s) { if (*s == c) return s; if (!*s) return nullptr; }
}
template <CodeUnit16 C> constexpr const C* strrchr(const C* s, C c) noexcept {
	const C* r = nullptr;
	for (;; ++s) { if (*s == c) r = s; if (!*s) return r; }
}
template <CodeUnit16 C> constexpr const C* strstr(const C* s, const C* sub) noexcept {
	if (!*sub) return s;
	const size_t n = strlen(sub);
	for (; *s; ++s) if (*s == *sub && strncmp(s, sub, n) == 0) return s;
	return nullptr;
}
template <CodeUnit16 C> constexpr size_t strspn(const C* s, const C* set) noexcept {
	size_t n = 0; while (s[n] && strchr(set, s[n])) ++n; return n;
}
template <CodeUnit16 C> constexpr size_t strcspn(const C* s, const C* set) noexcept {
	size_t n = 0; while (s[n] && !strchr(set, s[n])) ++n; return n;
}
template <CodeUnit16 C> constexpr const C* strpbrk(const C* s, const C* set) noexcept {
	s += strcspn(s, set); return *s ? s : nullptr;
}
template <CodeUnit16 C> constexpr C* strcpy(C* d, const C* s) noexcept { C* r = d; while ((*d++ = *s++)) {} return r; }
//! C 標準と同じく、src が短いときは残りを 0 で埋め、長いときは終端しない
template <CodeUnit16 C> constexpr C* strncpy(C* d, const C* s, size_t n) noexcept {
	size_t i = 0;
	for (; i < n && s[i]; ++i) d[i] = s[i];
	for (; i < n; ++i) d[i] = 0;
	return d;
}
template <CodeUnit16 C> constexpr C* strcat(C* d, const C* s) noexcept { strcpy(d + strlen(d), s); return d; }
template <CodeUnit16 C> constexpr C* strncat(C* d, const C* s, size_t n) noexcept {
	C* p = d + strlen(d);
	while (n-- && *s) *p++ = *s++;
	*p = 0;
	return d;
}
template <CodeUnit16 C> inline int memcmp(const C* a, const C* b, size_t n) noexcept {
	for (; n; --n, ++a, ++b) {
		const auto x = static_cast<std::uint16_t>(*a), y = static_cast<std::uint16_t>(*b);
		if (x != y) return x < y ? -1 : 1;
	}
	return 0;
}
template <CodeUnit16 C> inline C* memcpy(C* d, const C* s, size_t n) noexcept { if (n) std::memcpy(d, s, n * sizeof(C)); return d; }
template <CodeUnit16 C> inline C* memmove(C* d, const C* s, size_t n) noexcept { if (n) std::memmove(d, s, n * sizeof(C)); return d; }
template <CodeUnit16 C> constexpr C* memset(C* d, C c, size_t n) noexcept { for (size_t i = 0; i < n; ++i) d[i] = c; return d; }
template <CodeUnit16 C> constexpr const C* memchr(const C* s, C c, size_t n) noexcept {
	for (; n; --n, ++s) if (*s == c) return s;
	return nullptr;
}
//! _wcsdup 相当。free() で解放する。
template <CodeUnit16 C> inline C* strdup(const C* s) noexcept {
	const size_t n = strlen(s) + 1;
	auto p = static_cast<C*>(std::malloc(n * sizeof(C)));
	if (p) memcpy(p, s, n);
	return p;
}
//! wcstok_s 相当
template <CodeUnit16 C> constexpr C* strtok(C* s, const C* delim, C** context) noexcept {
	if (!s) s = *context;
	if (!s) return nullptr;
	s += strspn(s, delim);
	if (!*s) { *context = s; return nullptr; }
	C* end = s + strcspn(s, delim);
	if (*end) { *end = 0; *context = end + 1; } else { *context = end; }
	return s;
}

// ---------------------------------------------------------------------------
// 数値変換（wcstol / _wtoi / _itow 相当）
// 数値に使える文字は ASCII だけなので、一時的に char に写して C の関数に任せる。
// ---------------------------------------------------------------------------
namespace detail {
template <CodeUnit16 C, typename F>
inline auto narrow_call(const C* s, C** end, F&& f) {
	char buf[128];
	size_t n = 0;
	for (; n + 1 < sizeof(buf) && s[n] && static_cast<std::uint16_t>(s[n]) < 0x80; ++n) buf[n] = static_cast<char>(s[n]);
	buf[n] = '\0';
	char* e = buf;
	errno = 0;
	auto v = f(buf, &e);
	if (end) *end = const_cast<C*>(s + (e - buf));
	return v;
}
} // namespace detail

template <CodeUnit16 C> inline long strtol(const C* s, C** end, int base) {
	return detail::narrow_call(s, end, [base](const char* b, char** e) { return std::strtol(b, e, base); });
}
template <CodeUnit16 C> inline unsigned long strtoul(const C* s, C** end, int base) {
	return detail::narrow_call(s, end, [base](const char* b, char** e) { return std::strtoul(b, e, base); });
}
template <CodeUnit16 C> inline long long strtoll(const C* s, C** end, int base) {
	return detail::narrow_call(s, end, [base](const char* b, char** e) { return std::strtoll(b, e, base); });
}
template <CodeUnit16 C> inline double strtod(const C* s, C** end) {
	return detail::narrow_call(s, end, [](const char* b, char** e) { return std::strtod(b, e); });
}
//! _wtoi 相当（範囲外は Windows と同じく INT_MAX / INT_MIN に丸める）
template <CodeUnit16 C> inline int atoi(const C* s) {
	const long long v = strtoll(s, static_cast<C**>(nullptr), 10);
	if (v > INT_MAX) return INT_MAX;
	if (v < INT_MIN) return INT_MIN;
	return static_cast<int>(v);
}
template <CodeUnit16 C> inline long atol(const C* s) {
	const long long v = strtoll(s, static_cast<C**>(nullptr), 10);
	if (v > INT32_MAX) return INT32_MAX;
	if (v < INT32_MIN) return INT32_MIN;
	return static_cast<long>(v);
}
//! _itow / _i64tow 相当の共通部分：u を基数 radix（2〜36）で書き出す。neg なら先頭に負号。
template <CodeUnit16 C> inline C* utoa(unsigned long long u, bool neg, C* buf, int radix) noexcept {
	if (radix < 2 || radix > 36) { buf[0] = 0; return buf; }
	C tmp[72]; size_t n = 0;
	do { const int d = static_cast<int>(u % static_cast<unsigned>(radix)); tmp[n++] = static_cast<C>(d < 10 ? '0' + d : 'a' + d - 10); u /= static_cast<unsigned>(radix); } while (u);
	size_t i = 0;
	if (neg) buf[i++] = C('-');
	while (n) buf[i++] = tmp[--n];
	buf[i] = 0;
	return buf;
}
//! _itow / _ltow 相当（32 ビット。基数 10 のときだけ負号、それ以外は 2 の補数として書く）
template <CodeUnit16 C> inline C* itoa32(std::int32_t v, C* buf, int radix) noexcept {
	const bool neg = (radix == 10 && v < 0);
	const unsigned long long u = neg ? 0ULL - static_cast<unsigned long long>(static_cast<long long>(v)) : static_cast<std::uint32_t>(v);
	return utoa(u, neg, buf, radix);
}
//! _i64tow 相当（64 ビット）
template <CodeUnit16 C> inline C* itoa64(std::int64_t v, C* buf, int radix) noexcept {
	const bool neg = (radix == 10 && v < 0);
	const unsigned long long u = neg ? 0ULL - static_cast<unsigned long long>(v) : static_cast<unsigned long long>(v);
	return utoa(u, neg, buf, radix);
}

// ---------------------------------------------------------------------------
// UTF 変換
// ---------------------------------------------------------------------------
//! UTF-16 → UTF-32 で 1 文字読む。不正なサロゲートは U+FFFD。
template <CodeUnit16 C>
constexpr char32_t decode_one(const C*& p, const C* end) noexcept {
	const char32_t c = static_cast<std::uint16_t>(*p++);
	if (is_high_surrogate(c)) {
		if (p != end && is_low_surrogate(static_cast<std::uint16_t>(*p))) {
			const char32_t l = static_cast<std::uint16_t>(*p++);
			return 0x10000 + ((c - 0xD800) << 10) + (l - 0xDC00);
		}
		return 0xFFFD;
	}
	return is_low_surrogate(c) ? 0xFFFD : c;
}
template <typename String>
inline void append_utf16(String& out, char32_t c) {
	using C = typename String::value_type;
	if (c > 0x10FFFF || is_high_surrogate(c) || is_low_surrogate(c)) c = 0xFFFD;
	if (c >= 0x10000) {
		c -= 0x10000;
		out.push_back(static_cast<C>(0xD800 + (c >> 10)));
		out.push_back(static_cast<C>(0xDC00 + (c & 0x3FF)));
	} else {
		out.push_back(static_cast<C>(c));
	}
}
template <CodeUnit16 C>
inline std::string to_utf8(std::basic_string_view<C> s) {
	std::string out;
	out.reserve(s.size());
	for (const C* p = s.data(), *e = p + s.size(); p != e;) {
		const char32_t c = decode_one(p, e);
		if (c < 0x80) out.push_back(static_cast<char>(c));
		else if (c < 0x800) { out.push_back(static_cast<char>(0xC0 | (c >> 6))); out.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
		else if (c < 0x10000) { out.push_back(static_cast<char>(0xE0 | (c >> 12))); out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); out.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
		else { out.push_back(static_cast<char>(0xF0 | (c >> 18))); out.push_back(static_cast<char>(0x80 | ((c >> 12) & 0x3F))); out.push_back(static_cast<char>(0x80 | ((c >> 6) & 0x3F))); out.push_back(static_cast<char>(0x80 | (c & 0x3F))); }
	}
	return out;
}
inline std::string to_utf8(const WCHAR* s) { return to_utf8(std::basic_string_view<WCHAR>(s)); }
//! UTF-8 → UTF-16。不正なバイト列は U+FFFD。
template <CodeUnit16 C = WCHAR>
inline std::basic_string<C> from_utf8(std::string_view s) {
	std::basic_string<C> out;
	out.reserve(s.size());
	const auto* p = reinterpret_cast<const unsigned char*>(s.data());
	const auto* e = p + s.size();
	while (p != e) {
		const unsigned char b = *p++;
		char32_t c; int extra; char32_t min;
		if (b < 0x80) { out.push_back(static_cast<C>(b)); continue; }
		else if ((b & 0xE0) == 0xC0) { c = b & 0x1F; extra = 1; min = 0x80; }
		else if ((b & 0xF0) == 0xE0) { c = b & 0x0F; extra = 2; min = 0x800; }
		else if ((b & 0xF8) == 0xF0) { c = b & 0x07; extra = 3; min = 0x10000; }
		else { append_utf16(out, 0xFFFD); continue; }
		int i = 0;
		for (; i < extra && p != e && (*p & 0xC0) == 0x80; ++i) c = (c << 6) | (*p++ & 0x3F);
		append_utf16(out, (i != extra || c < min) ? char32_t(0xFFFD) : c);
	}
	return out;
}
//! UTF-16 → wchar_t の文字列（Windows では同じ、macOS では UTF-32）。std::filesystem などとの橋渡し用。
template <CodeUnit16 C>
inline std::wstring to_native_wide(std::basic_string_view<C> s) {
	std::wstring out;
	if constexpr (sizeof(wchar_t) == 2) {
		out.assign(reinterpret_cast<const wchar_t*>(s.data()), s.size());
	} else {
		out.reserve(s.size());
		for (const C* p = s.data(), *e = p + s.size(); p != e;) out.push_back(static_cast<wchar_t>(decode_one(p, e)));
	}
	return out;
}
template <CodeUnit16 C = WCHAR>
inline std::basic_string<C> from_native_wide(std::wstring_view s) {
	std::basic_string<C> out;
	if constexpr (sizeof(wchar_t) == 2) {
		out.assign(reinterpret_cast<const C*>(s.data()), s.size());
	} else {
		out.reserve(s.size());
		for (wchar_t w : s) append_utf16(out, static_cast<char32_t>(w));
	}
	return out;
}

// ---------------------------------------------------------------------------
// printf（MSVC の wide printf と同じ書式を解釈する）
//   %s %ls %ws : UTF-16 文字列     %hs %S : char 文字列（UTF-8 として扱う）
//   %c %lc %wc : UTF-16 文字       %hc %C : char 文字
//   %I64d %I32d %Id %zd %lld など、整数・浮動小数点・%p・%% に対応。%n は未対応（MSVC 既定と同じ）。
// 戻り値は書式化後の長さ（終端 NUL を含まない）。buf が小さいときは切り詰めて NUL 終端する。
// 書式エラーのときは -1。
// ---------------------------------------------------------------------------
template <CodeUnit16 C>
inline int vformat(C* buf, size_t size, const C* fmt, va_list ap) {
	size_t pos = 0;
	auto put = [&](C c) { if (pos + 1 < size) buf[pos] = c; ++pos; };
	auto pad = [&](size_t n, C c) { while (n--) put(c); };
	for (const C* f = fmt; *f; ++f) {
		if (*f != C('%')) { put(*f); continue; }
		++f;
		if (*f == C('%')) { put(C('%')); continue; }
		// flags
		std::string spec = "%";
		bool left = false;
		for (; *f == C('-') || *f == C('+') || *f == C(' ') || *f == C('#') || *f == C('0'); ++f) {
			if (*f == C('-')) left = true;
			spec.push_back(static_cast<char>(*f));
		}
		// width
		int width = -1;
		if (*f == C('*')) { width = va_arg(ap, int); if (width < 0) { left = true; spec.push_back('-'); width = -width; } ++f; }
		else if (is_digit(*f)) { width = 0; for (; is_digit(*f); ++f) width = width * 10 + (*f - C('0')); }
		if (width >= 0) spec += std::to_string(width);
		// precision
		int prec = -1;
		if (*f == C('.')) {
			++f; prec = 0;
			if (*f == C('*')) { prec = va_arg(ap, int); ++f; }
			else for (; is_digit(*f); ++f) prec = prec * 10 + (*f - C('0'));
			if (prec >= 0) { spec.push_back('.'); spec += std::to_string(prec); }
		}
		// length
		enum Len { none, hh, h, l, ll, j, z, t, L, wide } len = none;
		if (*f == C('h')) { ++f; len = h; if (*f == C('h')) { ++f; len = hh; } }
		else if (*f == C('l')) { ++f; len = l; if (*f == C('l')) { ++f; len = ll; } }
		else if (*f == C('w')) { ++f; len = wide; }
		else if (*f == C('j')) { ++f; len = j; }
		else if (*f == C('z')) { ++f; len = z; }
		else if (*f == C('t')) { ++f; len = t; }
		else if (*f == C('L')) { ++f; len = L; }
		else if (*f == C('I')) {
			++f;
			if (f[0] == C('6') && f[1] == C('4')) { f += 2; len = ll; }
			else if (f[0] == C('3') && f[1] == C('2')) { f += 2; len = none; }
			else len = z;
		}
		const C conv = *f;
		if (!conv) return -1;
		auto emit_units = [&](auto&& units, size_t n) {
			const size_t w = width > 0 ? static_cast<size_t>(width) : 0;
			if (!left && w > n) pad(w - n, C(' '));
			for (size_t i = 0; i < n; ++i) put(units(i));
			if (left && w > n) pad(w - n, C(' '));
		};
		switch (conv) {
		case C('s'): case C('S'): {
			const bool narrow = (conv == C('S')) ? (len != l && len != wide) : (len == h);
			if (narrow) {
				const char* s = va_arg(ap, const char*);
				if (!s) s = "(null)";
				std::string_view sv(s);
				if (prec >= 0 && sv.size() > static_cast<size_t>(prec)) sv = sv.substr(0, prec);
				const auto w = from_utf8<C>(sv);
				emit_units([&](size_t i) { return w[i]; }, w.size());
			} else {
				const C* s = va_arg(ap, const C*);
				static constexpr C null_str[] = { C('('), C('n'), C('u'), C('l'), C('l'), C(')'), C(0) };
				if (!s) s = null_str;
				const size_t n = prec >= 0 ? strnlen(s, static_cast<size_t>(prec)) : strlen(s);
				emit_units([&](size_t i) { return s[i]; }, n);
			}
			break;
		}
		case C('c'): case C('C'): {
			const bool narrow = (conv == C('C')) ? (len != l && len != wide) : (len == h);
			const int v = va_arg(ap, int);
			const C ch = narrow ? static_cast<C>(static_cast<unsigned char>(v)) : static_cast<C>(v);
			emit_units([&](size_t) { return ch; }, 1);
			break;
		}
		case C('d'): case C('i'): case C('u'): case C('o'): case C('x'): case C('X'):
		case C('e'): case C('E'): case C('f'): case C('F'): case C('g'): case C('G'):
		case C('a'): case C('A'): case C('p'): {
			char out[512];
			int n;
			const char cc = static_cast<char>(conv);
			const bool is_signed = (cc == 'd' || cc == 'i');
			if (cc == 'p') {
				spec += 'p';
				n = std::snprintf(out, sizeof(out), spec.c_str(), va_arg(ap, void*));
			} else if (std::strchr("eEfFgGaA", cc)) {
				if (len == L) { spec += 'L'; spec += cc; n = std::snprintf(out, sizeof(out), spec.c_str(), va_arg(ap, long double)); }
				else { spec += cc; n = std::snprintf(out, sizeof(out), spec.c_str(), va_arg(ap, double)); }
			} else {
				spec += "ll"; spec += cc;
				long long sv = 0; unsigned long long uv = 0;
				switch (len) {
				case hh: if (is_signed) sv = static_cast<signed char>(va_arg(ap, int)); else uv = static_cast<unsigned char>(va_arg(ap, unsigned int)); break;
				case h:  if (is_signed) sv = static_cast<short>(va_arg(ap, int)); else uv = static_cast<unsigned short>(va_arg(ap, unsigned int)); break;
				case l:  if (is_signed) sv = va_arg(ap, long); else uv = va_arg(ap, unsigned long); break;
				case ll: if (is_signed) sv = va_arg(ap, long long); else uv = va_arg(ap, unsigned long long); break;
				case j:  if (is_signed) sv = va_arg(ap, std::intmax_t); else uv = va_arg(ap, std::uintmax_t); break;
				case z:  if (is_signed) sv = static_cast<long long>(va_arg(ap, std::ptrdiff_t)); else uv = va_arg(ap, std::size_t); break;
				case t:  if (is_signed) sv = va_arg(ap, std::ptrdiff_t); else uv = static_cast<unsigned long long>(va_arg(ap, std::ptrdiff_t)); break;
				default: if (is_signed) sv = va_arg(ap, int); else uv = va_arg(ap, unsigned int); break;
				}
				n = is_signed ? std::snprintf(out, sizeof(out), spec.c_str(), sv) : std::snprintf(out, sizeof(out), spec.c_str(), uv);
			}
			if (n < 0) return -1;
			for (int i = 0; i < n && i + 1 < static_cast<int>(sizeof(out)); ++i) put(static_cast<C>(static_cast<unsigned char>(out[i])));
			break;
		}
		default:
			return -1; // %n や未知の変換
		}
	}
	if (size) buf[pos < size ? pos : size - 1] = 0;
	return pos > static_cast<size_t>(INT_MAX) ? -1 : static_cast<int>(pos);
}
template <CodeUnit16 C>
inline int format(C* buf, size_t size, const C* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = vformat(buf, size, fmt, ap); va_end(ap); return n;
}
//! 書式化後の長さだけを求める（_vscwprintf 相当）
template <CodeUnit16 C>
inline int vcount(const C* fmt, va_list ap) { C dummy[1]; return vformat(dummy, 0, fmt, ap); }

//! _vsnwprintf_s 相当（MSVC と同じ戻り値・切り詰め規則）
template <CodeUnit16 C>
inline int vsnprintf_s(C* buf, size_t size, size_t count, const C* fmt, va_list ap) {
	if (!buf || size == 0 || !fmt) { errno = EINVAL; return -1; }
	const bool truncate = (count == static_cast<size_t>(-1));
	const size_t limit = truncate ? size : (count < size ? count + 1 : size);
	va_list ap2; va_copy(ap2, ap);
	const int n = vformat(buf, limit, fmt, ap2);
	va_end(ap2);
	if (n < 0) { buf[0] = 0; errno = EINVAL; return -1; }
	if (static_cast<size_t>(n) < limit) return n;
	// 収まらなかった
	if (truncate) return -1;                       // 切り詰めた文字列を残す
	if (count < size) return -1;                   // count で切った（MSVC と同じく -1）
	buf[0] = 0; errno = ERANGE; return -1;         // バッファ不足
}

} // namespace u16
} // namespace sakura

// ---------------------------------------------------------------------------
// 非 Windows 平台：char16_t 用の MSVC 互換オーバーロード
// ---------------------------------------------------------------------------
#if SAKURA_WCHAR_IS_CHAR16
namespace su16 = sakura::u16;

inline size_t   wcslen(const char16_t* s) noexcept { return su16::strlen(s); }
inline size_t   wcsnlen(const char16_t* s, size_t n) noexcept { return su16::strnlen(s, n); }
inline int      wcscmp(const char16_t* a, const char16_t* b) noexcept { return su16::strcmp(a, b); }
inline int      wcsncmp(const char16_t* a, const char16_t* b, size_t n) noexcept { return su16::strncmp(a, b, n); }
inline int      _wcsicmp(const char16_t* a, const char16_t* b) noexcept { return su16::stricmp(a, b); }
inline int      _wcsnicmp(const char16_t* a, const char16_t* b, size_t n) noexcept { return su16::strnicmp(a, b, n); }
inline int      wcsicmp(const char16_t* a, const char16_t* b) noexcept { return su16::stricmp(a, b); }
inline const char16_t* wcschr(const char16_t* s, char16_t c) noexcept { return su16::strchr(s, c); }
inline       char16_t* wcschr(char16_t* s, char16_t c) noexcept { return const_cast<char16_t*>(su16::strchr<char16_t>(s, c)); }
inline const char16_t* wcsrchr(const char16_t* s, char16_t c) noexcept { return su16::strrchr(s, c); }
inline       char16_t* wcsrchr(char16_t* s, char16_t c) noexcept { return const_cast<char16_t*>(su16::strrchr<char16_t>(s, c)); }
inline const char16_t* wcsstr(const char16_t* s, const char16_t* t) noexcept { return su16::strstr(s, t); }
inline       char16_t* wcsstr(char16_t* s, const char16_t* t) noexcept { return const_cast<char16_t*>(su16::strstr<char16_t>(s, t)); }
inline const char16_t* wcspbrk(const char16_t* s, const char16_t* t) noexcept { return su16::strpbrk(s, t); }
inline       char16_t* wcspbrk(char16_t* s, const char16_t* t) noexcept { return const_cast<char16_t*>(su16::strpbrk<char16_t>(s, t)); }
inline size_t   wcsspn(const char16_t* s, const char16_t* t) noexcept { return su16::strspn(s, t); }
inline size_t   wcscspn(const char16_t* s, const char16_t* t) noexcept { return su16::strcspn(s, t); }
inline char16_t* wcscpy(char16_t* d, const char16_t* s) noexcept { return su16::strcpy(d, s); }
inline char16_t* wcsncpy(char16_t* d, const char16_t* s, size_t n) noexcept { return su16::strncpy(d, s, n); }
inline char16_t* wcscat(char16_t* d, const char16_t* s) noexcept { return su16::strcat(d, s); }
inline char16_t* wcsncat(char16_t* d, const char16_t* s, size_t n) noexcept { return su16::strncat(d, s, n); }
inline char16_t* _wcsdup(const char16_t* s) noexcept { return su16::strdup(s); }
inline char16_t* wcstok_s(char16_t* s, const char16_t* delim, char16_t** ctx) noexcept { return su16::strtok(s, delim, ctx); }
inline char16_t* _wcstok(char16_t* s, const char16_t* delim) noexcept { thread_local char16_t* ctx = nullptr; return su16::strtok(s, delim, &ctx); }
inline char16_t* _wcslwr(char16_t* s) noexcept { for (auto p = s; *p; ++p) *p = su16::to_lower(*p); return s; }
inline char16_t* _wcsupr(char16_t* s) noexcept { for (auto p = s; *p; ++p) *p = su16::to_upper(*p); return s; }

inline int       wmemcmp(const char16_t* a, const char16_t* b, size_t n) noexcept { return su16::memcmp(a, b, n); }
inline char16_t* wmemcpy(char16_t* d, const char16_t* s, size_t n) noexcept { return su16::memcpy(d, s, n); }
inline char16_t* wmemmove(char16_t* d, const char16_t* s, size_t n) noexcept { return su16::memmove(d, s, n); }
inline char16_t* wmemset(char16_t* d, char16_t c, size_t n) noexcept { return su16::memset(d, c, n); }
inline const char16_t* wmemchr(const char16_t* s, char16_t c, size_t n) noexcept { return su16::memchr(s, c, n); }
inline       char16_t* wmemchr(char16_t* s, char16_t c, size_t n) noexcept { return const_cast<char16_t*>(su16::memchr<char16_t>(s, c, n)); }

inline long          wcstol(const char16_t* s, char16_t** e, int base) { return su16::strtol(s, e, base); }
inline unsigned long wcstoul(const char16_t* s, char16_t** e, int base) { return su16::strtoul(s, e, base); }
inline long long     wcstoll(const char16_t* s, char16_t** e, int base) { return su16::strtoll(s, e, base); }
inline long long     _wcstoi64(const char16_t* s, char16_t** e, int base) { return su16::strtoll(s, e, base); }
inline double        wcstod(const char16_t* s, char16_t** e) { return su16::strtod(s, e); }
inline int           _wtoi(const char16_t* s) { return su16::atoi(s); }
inline long          _wtol(const char16_t* s) { return su16::atol(s); }
inline double        _wtof(const char16_t* s) { return su16::strtod(s, static_cast<char16_t**>(nullptr)); }
inline char16_t* _itow(int v, char16_t* buf, int radix) noexcept { return su16::itoa32(static_cast<std::int32_t>(v), buf, radix); }
inline char16_t* _ltow(long v, char16_t* buf, int radix) noexcept { return su16::itoa32(static_cast<std::int32_t>(v), buf, radix); }
inline char16_t* _i64tow(long long v, char16_t* buf, int radix) noexcept { return su16::itoa64(static_cast<std::int64_t>(v), buf, radix); }

inline int vswprintf(char16_t* buf, size_t count, const char16_t* fmt, va_list ap) {
	const int n = su16::vformat(buf, count, fmt, ap);
	return (n >= 0 && static_cast<size_t>(n) < count) ? n : -1;
}
inline int swprintf(char16_t* buf, size_t count, const char16_t* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = vswprintf(buf, count, fmt, ap); va_end(ap); return n;
}
//! MSVC の旧形式（バッファサイズを取らない）
inline int _vswprintf(char16_t* buf, const char16_t* fmt, va_list ap) { return su16::vformat(buf, static_cast<size_t>(INT_MAX), fmt, ap); }
inline int _swprintf(char16_t* buf, const char16_t* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = _vswprintf(buf, fmt, ap); va_end(ap); return n;
}
inline int _vsnwprintf_s(char16_t* buf, size_t size, size_t count, const char16_t* fmt, va_list ap) { return su16::vsnprintf_s(buf, size, count, fmt, ap); }
inline int _snwprintf_s(char16_t* buf, size_t size, size_t count, const char16_t* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = _vsnwprintf_s(buf, size, count, fmt, ap); va_end(ap); return n;
}
inline int _vscwprintf(const char16_t* fmt, va_list ap) { return su16::vcount(fmt, ap); }
inline int _scwprintf(const char16_t* fmt, ...) {
	va_list ap; va_start(ap, fmt); const int n = _vscwprintf(fmt, ap); va_end(ap); return n;
}
#endif // SAKURA_WCHAR_IS_CHAR16

#endif /* SAKURA_WCHAR16_H_ */
