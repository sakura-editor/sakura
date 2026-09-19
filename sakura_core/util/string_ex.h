/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STRING_EX_87282FEB_4B23_4112_9C5A_419F43618705_H_
#define SAKURA_STRING_EX_87282FEB_4B23_4112_9C5A_419F43618705_H_
#pragma once

#include "basis/primitive.h"

#include <vadefs.h>
#include <string>
#include <string_view>
#include <variant>

#include "debug/Debug2.h"

// 2007.10.19 kobake
// string.h で定義されている関数を拡張したようなモノ達

/*
	++ ++ 命名参考(規則では無い) ++ ++

	標準関数から引用
	～_s:  バッファオーバーフロー考慮版 (例: strcpy_s)
	～i～: 大文字小文字区別無し版       (例: stricmp)

	独自
	auto_～:  引数の型により、自動で処理が決定される版 (例: auto_strcpy)
*/

namespace cxx {

/*!
 * @brief NUL終端文字列の文字列長を調べる
 *
 * C++標準だと上限を指定できないので自作。（Wide/Ascii両対応）
 *
 * @param[in] pText 文字列
 * @param[in] cchText 文字列バッファのサイズ
 * @returns 文字列長
 */
template <typename CharT>
constexpr size_t strnlen(
	const CharT* pText,
	size_t cchText
)
{
	const auto nulPos = std::char_traits<CharT>::find(pText, cchText, CharT());
	return nulPos ? static_cast<size_t>(nulPos - pText) : cchText - 1;
}

/*!
 * @brief 配列内のNUL終端位置を調べる
 *
 * 配列内で最初に現われるNULの位置を返す。
 *
 * @tparam[in] CharT 文字の型
 * @tparam[in] N 配列サイズ
 * @param[in] str 文字列
 * @returns 文字列長
 */
template <typename CharT, size_t N>
constexpr size_t strnlen_s(
	const CharT(&str)[N]
)
{
	return cxx::strnlen(str, std::size(str));
}

/*!
 * @brief 文字数を指定して文字列をコピーする
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * @tparam[in] A1 コピー先バッファの型（固定長バッファとして扱える型）
 * @tparam[in] A2 コピー元文字列の型（NUL終端文字列を生成できる型）
 * @param[out] dst コピー先バッファ
 * @param[in] src コピー元文字列
 * @param[in] count コピーする文字数
 * @retval 0 成功
 * @retval STRUNCATE 切り詰め発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, basis::NullTerminatedStringConstructible<CharT> A2>
constexpr errno_t strncpy_s(A1& dst, const A2& src, size_t count) noexcept
{
	// 入力元をNUL終端文字列とみなす
	const auto szText = cxx::NullTerminatedString{ src };

	// 入力元を文字列として扱う
	auto text = std::basic_string_view<CharT>{ szText };

	// 上限が指定されていない場合、入力元文字列の文字数を使う
	if (_TRUNCATE == count) {
		count = text.length();
	}

	// 切り詰めが発生したかどうかを記録するフラグ
	bool truncated = false;

	// 出力先を固定長バッファとして扱う
	auto buffer = std::span<CharT>{ dst };
	
	// 上限がバッファサイズを超えていた場合、バッファサイズに切り詰める
	if (const auto bufferSize = std::size(buffer);
		bufferSize <= count)
	{
		count = bufferSize - 1;

		truncated = true;
	}

	// 文字列をコピーする
	std::ranges::copy_n(text.begin(), count, buffer.begin());
	std::char_traits<CharT>::assign(buffer[count], CharT());

	// 切り詰めが発生したかどうかに応じて固定値を返す
	return truncated ? STRUNCATE : 0;
}

/*!
 * @brief 文字列をコピーする
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 * 
 * @tparam[in] A1 コピー先バッファの型（固定長バッファとして扱える型）
 * @tparam[in] A2 コピー元文字列の型（NUL終端文字列を生成できる型）
 * @param[out] dst コピー先バッファ
 * @param[in] src コピー元文字列
 * @retval 0 成功
 * @retval STRUNCATE 切り詰め発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, basis::NullTerminatedStringConstructible<CharT> A2>
constexpr errno_t strcpy_s(A1& dst, const A2& src) noexcept
{
	return cxx::strncpy_s<CharT>(dst, src, _TRUNCATE);
}

/*!
 * @brief 文字数を指定して末尾に文字列をコピーする
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * @tparam[in] A1 コピー先バッファの型（固定長バッファとして扱える型）
 * @tparam[in] A2 コピー元文字列の型（NUL終端文字列を生成できる型）
 * @param[out] dst コピー先バッファ
 * @param[in] src コピー元文字列
 * @param[in] count コピーする文字数
 * @retval 0 成功
 * @retval STRUNCATE 切り詰め発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, basis::NullTerminatedStringConstructible<CharT> A2>
constexpr errno_t strncat_s(A1& dst, const A2& src, size_t count) noexcept
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span<CharT>{ dst };

	// 出力先に既に入っている文字列の長さを求める
	const auto len = cxx::strnlen(buffer.data(), buffer.size());

	// バッファの末尾から残りを切り出す
	buffer = buffer.subspan(len);

	// 切り出したバッファに文字列をコピーする
	return cxx::strncpy_s<CharT>(buffer, src, count);
}

/*!
 * @brief 末尾に文字列をコピーする
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * @tparam[in] A1 コピー先バッファの型（固定長バッファとして扱える型）
 * @tparam[in] A2 コピー元文字列の型（NUL終端文字列を生成できる型）
 * @param[out] dst コピー先バッファ
 * @param[in] src コピー元文字列
 * @retval 0 成功
 * @retval STRUNCATE 切り詰め発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, basis::NullTerminatedStringConstructible<CharT> A2>
constexpr errno_t strcat_s(A1& dst, const A2& src) noexcept
{
	return cxx::strncat_s<CharT>(dst, src, _TRUNCATE);
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
 // TODO: いつか廃止する
inline int _vsnprintf_s(
	WCHAR* pBuffer,
	size_t nBufferSIze,
	size_t nMaxCount,
	_In_z_ _Printf_format_string_ LPCWSTR format,
	va_list& argList
) noexcept
{
	return ::_vsnwprintf_s(
		pBuffer,
		nBufferSIze,
		nMaxCount,
		format,
		argList
	);
}

// TODO: いつか廃止する
inline int _vsnprintf_s(
	ACHAR* pBuffer,
	size_t nBufferSIze,
	size_t nMaxCount,
	_In_z_ _Printf_format_string_ LPCSTR format,
	va_list& argList
) noexcept
{
	return ::_vsnprintf_s(
		pBuffer,
		nBufferSIze,
		nMaxCount,
		format,
		argList
	);
}

// TODO: いつか廃止する
template <typename CharT, basis::WritableBuffer<CharT> A>
int _vsnprintf_s(
	A& dst,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& argList
) noexcept
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span<CharT>{ dst };
	
	// Cランタイムの実装に任せる
	return cxx::_vsnprintf_s(
		std::data(buffer),
		std::size(buffer),
		count,
		format,
		argList
	);
}

// TODO: いつか廃止する
inline int _vscprintf(
	_In_z_ _Printf_format_string_ LPCWSTR format,
	va_list& argList
) noexcept
{
	return ::_vscwprintf( format, argList );
}

// TODO: いつか廃止する
inline int _vscprintf(
	_In_z_ _Printf_format_string_ LPCSTR format,
	va_list& argList
) noexcept
{
	return ::_vscprintf( format, argList );
}

// TODO: いつか廃止する
template <typename CharT, basis::WritableBuffer<CharT> A>
int _vsprintf_s(
	A& dst,
	const CharT* format,
	va_list& argList
) noexcept
{
	return cxx::_vsnprintf_s( dst, _TRUNCATE, format, argList );
}

// TODO: いつか廃止する
template <typename CharT>
int _vsprintf_s(
	CharT* pBuffer,
	size_t nBufferSIze,
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& argList
) noexcept
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span{ pBuffer, nBufferSIze };

	return cxx::_vsprintf_s( buffer, format, argList );
}

/*!
 * @brief sprintf系関数の引数を変換する
 *
 * sprintf系関数にC++標準の文字列を渡せない対策として作成。
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 */
template <typename T>
constexpr decltype(auto) ConvertPrintfArg(const T& value)
{
	if constexpr (basis::NullTerminatedStringConstructible<T, WCHAR>) {
		const auto text = cxx::NullTerminatedString<WCHAR>{ value };
		if (text.uses_buffer()) {
			throw std::invalid_argument("invalid usage");
		}
		return text.c_str();
	}
	else if constexpr (basis::NullTerminatedStringConstructible<T, ACHAR>) {
		const auto text = cxx::NullTerminatedString<ACHAR>{ value };
		if (text.uses_buffer()) {
			throw std::invalid_argument("invalid usage");
		}
		return text.c_str();
	}
	else {
		return value;
	}
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <typename... Params>
int _snprintf_s(
	WCHAR* pBuffer,
	size_t nBufferSIze,
	size_t nMaxCount,
	_In_z_ _Printf_format_string_ const WCHAR* format,
	const Params&... params
)
{
	// Cランタイムの実装にパラメータ展開で転送する
	return ::_snwprintf_s(
		pBuffer,
		nBufferSIze,
		nMaxCount,
		format,
		cxx::ConvertPrintfArg(std::as_const(params))...
	);
}

template <typename... Params>
int _snprintf_s(
	ACHAR* pBuffer,
	size_t nBufferSIze,
	size_t nMaxCount,
	_In_z_ _Printf_format_string_ const ACHAR* format,
	const Params&... params
)
{
	// Cランタイムの実装にパラメータ展開で転送する
	return ::_snprintf_s(
		pBuffer,
		nBufferSIze,
		nMaxCount,
		format,
		cxx::ConvertPrintfArg(std::as_const(params))...
	);
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, typename... Params>
int _snprintf_s(
	A1& dst,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span{ dst };

	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		std::data(buffer),
		std::size(buffer),
		count,
		format,
		std::as_const(params)...
	);
}

/*!
 * @brief 書式付きデータの文字数をカウントします。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれる文字数
 * @retval < 0 エラー発生
 */
template <typename... Params>
int _scprintf(
	_In_z_ _Printf_format_string_ LPCWSTR format,
	const Params&... params
)
{
	return ::_scwprintf(
		format,
		cxx::ConvertPrintfArg(std::as_const(params))...
	);
}

template <typename... Params>
int _scprintf(
	_In_z_ _Printf_format_string_ LPCSTR format,
	const Params&... params
)
{
	return ::_scprintf(
		format,
		cxx::ConvertPrintfArg(std::as_const(params))...
	);
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 同名のCランタイム関数を自前実装したもの。（Wide/Ascii両対応）
 *
 * sprintf系関数はセキュリティ上問題あるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <typename CharT, basis::WritableBuffer<CharT> A1, typename... Params>
int _sprintf_s(
	A1& dst,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		dst,
		_TRUNCATE,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, typename... Params>
int _sprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// 出力先を固定長バッファとして扱う
	auto buffer = std::span{ pBuffer, nBufferSize };

	// パラメータ展開で転送する
	return cxx::_sprintf_s(
		buffer,
		format,
		std::as_const(params)...
	);
}

} // namespace cxx

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          メモリ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// 文字列コピーや文字列比較の際に、mem系関数が使われている箇所が多々ありますが、
// mem系関数はvoidポインタを受け取り、型チェックが行われないので危険です。
// ここに、型チェック付きのmem系互換の関数を作成しました。…と書いたけど、実際のプロトタイプはもっと下のほうに。。(auto_mem～)
// (※対象がメモリなので、そもそも文字という概念は無いが、
//    便宜上、ACHAR系では1バイト単位を、WCHAR系では2バイト単位を、
//    文字とみなして処理を行う、ということで)

//メモリ比較
inline int amemcmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return ::memcmp(p1,p2,count); }

//大文字小文字を区別せずにメモリ比較
inline int amemicmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return ::_memicmp(p1,p2,count); }
       int wmemicmp(const WCHAR* p1, const WCHAR* p2, size_t count);
       int wmemicmp(const WCHAR* p1, const WCHAR* p2 );
       int wmemicmp_ascii(const WCHAR* p1, const WCHAR* p2, size_t count);

//元の関数と同じシグニチャ版。
//文字列以外のメモリ処理でmem～系関数を使う場面では、この関数を使っておくと、意味合いがはっきりして良い。
inline void* memset_raw(void* dest, int c, size_t size){ return ::memset(dest,c,size); }
inline void* memcpy_raw(void* dest, const void* src, size_t size){ return ::memcpy(dest,src,size); }

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           文字                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//文字変換
inline int my_toupper( int c ){ return (((c) >= 'a') && ((c) <= 'z')) ? ((c) - 'a' + 'A') : (c); }
inline int my_tolower( int c ){ return (((c) >= 'A') && ((c) <= 'Z')) ? ((c) - 'A' + 'a') : (c); }
inline int my_towupper( int c ){ return (((c) >= L'a') && ((c) <= L'z')) ? ((c) - L'a' + L'A') : (c); }
inline int my_towlower( int c ){ return (((c) >= L'A') && ((c) <= L'Z')) ? ((c) - L'A' + L'a') : (c); }
inline wchar_t my_towupper2(wchar_t c) { return (wchar_t)my_towupper(c); }
inline wchar_t my_towlower2(wchar_t c) { return (wchar_t)my_towlower(c); }
int skr_towupper( int c );
int skr_towlower( int c );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           拡張・独自実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//大文字小文字を区別せずに文字列を検索
const WCHAR* wcsistr( const WCHAR* s1, const WCHAR* s2 );
const ACHAR* stristr( const ACHAR* s1, const ACHAR* s2 );
inline WCHAR* wcsistr( WCHAR* s1, const WCHAR* s2 ){ return const_cast<WCHAR*>(wcsistr(static_cast<const WCHAR*>(s1),s2)); }
inline ACHAR* stristr( ACHAR* s1, const ACHAR* s2 ){ return const_cast<ACHAR*>(stristr(static_cast<const ACHAR*>(s1),s2)); }

//大文字小文字を区別せずに文字列を検索（日本語対応版）
const char* strchr_j(const char* s1, char c);				//!< strchr の日本語対応版。
const char* strichr_j( const char* s1, char c );			//!< strchr の大文字小文字同一視＆日本語対応版。
const char* strstr_j(const char* s1, const char* s2);		//!< strstr の日本語対応版。
const char* stristr_j( const char* s1, const char* s2 );	//!< strstr の大文字小文字同一視＆日本語対応版。
inline char* strchr_j ( char* s1, char c         ){ return const_cast<char*>(strchr_j ((const char*)s1, c )); }
inline char* strichr_j( char* s1, char c         ){ return const_cast<char*>(strichr_j((const char*)s1, c )); }
inline char* strstr_j ( char* s1, const char* s2 ){ return const_cast<char*>(strstr_j ((const char*)s1, s2)); }
inline char* stristr_j( char* s1, const char* s2 ){ return const_cast<char*>(stristr_j((const char*)s1, s2)); }

template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	//[in] 文字列バッファ(終端があること)
	int					nLen,		//[in] 文字列の長さ
	int*				pnOffset,	//[in,out] オフセット
	const CHAR_TYPE*	pDelimiter	//[in] 区切り文字
);

// ▽ シグニチャおよび動作仕様は変わらないけど、
// コンパイラと言語指定によって不正動作をしてしまうことを回避するために
// 独自に実装し直したもの。
int my_stricmp( const char *s1, const char *s2 );
int my_strnicmp( const char *s1, const char *s2, size_t n );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//        auto系（_UNICODE 定義に依存しない関数）              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//char型にするかwchar_t型にするか確定しない変数があります。
//下記関数群を使って文字列操作を行った場合、
//将来、その変数の型が変わっても、その操作箇所を書き直さなくても
//済むことになります。
//
//強制キャストによる使用は推奨しません。
//そもそも、この関数呼び出しに限らず、強制キャストは最低限に留めてください。
//せっかくの、C++の厳格な型チェックの恩恵を受けることができなくなります。

//転送系
inline ACHAR* auto_memcpy(ACHAR* dest, const ACHAR* src, size_t count){        ::memcpy (dest,src,count); return dest; }
inline WCHAR* auto_memcpy(WCHAR* dest, const WCHAR* src, size_t count){ return ::wmemcpy(dest,src,count);              }
inline ACHAR* auto_strcpy(ACHAR* dst, const ACHAR* src){ return strcpy(dst,src); }
inline WCHAR* auto_strcpy(WCHAR* dst, const WCHAR* src){ return wcscpy(dst,src); }
inline ACHAR* auto_strncpy(ACHAR* dst,const ACHAR* src,size_t count){ return strncpy(dst,src,count); }
inline WCHAR* auto_strncpy(WCHAR* dst,const WCHAR* src,size_t count){ return wcsncpy(dst,src,count); }
inline ACHAR* auto_memset(ACHAR* dest, ACHAR c, size_t count){        memset (dest,c,count); return dest; }
inline WCHAR* auto_memset(WCHAR* dest, WCHAR c, size_t count){ return wmemset(dest,c,count);              }
inline ACHAR* auto_strcat(ACHAR* dst, const ACHAR* src){ return strcat(dst,src); }
inline WCHAR* auto_strcat(WCHAR* dst, const WCHAR* src){ return wcscat(dst,src); }

template <basis::WritableBuffer<ACHAR> A1, basis::NullTerminatedStringConstructible<ACHAR> A2> constexpr errno_t auto_strncpy_s(A1& dst, const A2& src, size_t count) { return cxx::strncpy_s<ACHAR>(dst, src, count); }
template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t auto_strncpy_s(A1& dst, const A2& src, size_t count) { return cxx::strncpy_s<WCHAR>(dst, src, count); }
template <basis::WritableBuffer<ACHAR> A1, basis::NullTerminatedStringConstructible<ACHAR> A2> constexpr errno_t auto_strncat_s(A1& dst, const A2& src, size_t count) { return cxx::strncat_s<ACHAR>(dst, src, count); }
template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t auto_strncat_s(A1& dst, const A2& src, size_t count) { return cxx::strncat_s<WCHAR>(dst, src, count); }

template <basis::WritableBuffer<ACHAR> A1, basis::NullTerminatedStringConstructible<ACHAR> A2> constexpr errno_t auto_strcpy_s(A1& dst, const A2& src) { return auto_strncpy_s(dst, src, _TRUNCATE); }
template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t auto_strcpy_s(A1& dst, const A2& src) { return auto_strncpy_s(dst, src, _TRUNCATE); }
template <basis::WritableBuffer<ACHAR> A1, basis::NullTerminatedStringConstructible<ACHAR> A2> constexpr errno_t auto_strcat_s(A1& dst, const A2& src) { return auto_strncat_s(dst, src, _TRUNCATE); }
template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t auto_strcat_s(A1& dst, const A2& src) { return auto_strncat_s(dst, src, _TRUNCATE); }

template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t wcsncpy_s(A1& dst, const A2& src, size_t count) { return auto_strncpy_s(dst, src, count); }
template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t wcsncat_s(A1& dst, const A2& src, size_t count) { return auto_strncat_s(dst, src, count); }

template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t wcscpy_s(A1& dst, const A2& src) { return auto_strcpy_s(dst, src); }
template <basis::WritableBuffer<WCHAR> A1, basis::NullTerminatedStringConstructible<WCHAR> A2> constexpr errno_t wcscat_s(A1& dst, const A2& src) { return auto_strcat_s(dst, src); }

//比較系
inline int auto_memcmp (const ACHAR* p1, const ACHAR* p2, size_t count){ return amemcmp(p1,p2,count); }
inline int auto_memcmp (const WCHAR* p1, const WCHAR* p2, size_t count){ return wmemcmp(p1,p2,count); }
inline int auto_strcmp (const ACHAR* p1, const ACHAR* p2){ return strcmp(p1,p2); }
inline int auto_strcmp (const WCHAR* p1, const WCHAR* p2){ return wcscmp(p1,p2); }
inline int auto_strncmp(const ACHAR* str1, const ACHAR* str2, size_t count){ return strncmp(str1,str2,count); }
inline int auto_strncmp(const WCHAR* str1, const WCHAR* str2, size_t count){ return wcsncmp(str1,str2,count); }

//比較系（ASCII, UCS2 専用）
inline int auto_memicmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return amemicmp(p1,p2,count); }
inline int auto_memicmp(const WCHAR* p1, const WCHAR* p2, size_t count){ return wmemicmp(p1,p2,count); }

//比較系（SJIS, UTF-16 専用)
inline int auto_strnicmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return my_strnicmp(p1,p2,count); }
inline int auto_strnicmp(const WCHAR* p1, const WCHAR* p2, size_t count){ return wmemicmp(p1,p2,count); } // Stub.
inline int auto_stricmp(const ACHAR* p1, const ACHAR* p2){ return my_stricmp(p1,p2); }
inline int auto_stricmp(const WCHAR* p1, const WCHAR* p2){ return wmemicmp(p1,p2); } // Stub.

//長さ計算系
inline size_t auto_strlen(const ACHAR* str){ return strlen(str); }
inline size_t auto_strlen(const WCHAR* str){ return wcslen(str); }

constexpr size_t auto_strnlen(const ACHAR* str, size_t count) { return cxx::strnlen(str, count); }
constexpr size_t auto_strnlen(const WCHAR* str, size_t count) { return cxx::strnlen(str, count); }

//検索系（SJIS, UCS2 専用）
inline const ACHAR* auto_strstr(const ACHAR* str, const ACHAR* strSearch){ return ::strstr_j(str,strSearch); }
inline const WCHAR* auto_strstr(const WCHAR* str, const WCHAR* strSearch){ return ::wcsstr  (str,strSearch); }
inline       ACHAR* auto_strstr(      ACHAR* str, const ACHAR* strSearch){ return ::strstr_j(str,strSearch); }
inline       WCHAR* auto_strstr(      WCHAR* str, const WCHAR* strSearch){ return ::wcsstr  (str,strSearch); }
inline const ACHAR* auto_strchr(const ACHAR* str, ACHAR c){ return ::strchr_j(str,c); }
inline const WCHAR* auto_strchr(const WCHAR* str, WCHAR c){ return ::wcschr  (str,c); }
inline       ACHAR* auto_strchr(      ACHAR* str, ACHAR c){ return ::strchr_j(str,c); }
inline       WCHAR* auto_strchr(      WCHAR* str, WCHAR c){ return ::wcschr  (str,c); }

//変換系
inline long auto_atol(const ACHAR* str){ return atol(str);  }
inline long auto_atol(const WCHAR* str){ return _wtol(str); }
ACHAR* tcstostr( ACHAR* dest, const WCHAR* src, size_t count );
WCHAR* tcstostr( WCHAR* dest, const WCHAR* src, size_t count );
WCHAR* strtotcs( WCHAR* dest, const ACHAR* src, size_t count );
WCHAR* strtotcs( WCHAR* dest, const WCHAR* src, size_t count );

//印字系
inline int auto_vsprintf(ACHAR* buf, const ACHAR* format, va_list& v) { return ::vsprintf(buf, format, v); }
inline int auto_vsprintf(WCHAR* buf, const WCHAR* format, va_list& v) { return ::_vswprintf(buf, format, v); }
inline int auto_sprintf(ACHAR* buf, const ACHAR* format, ...) { va_list args; va_start(args, format); const int n = auto_vsprintf(buf, format, args); va_end(args); return n; }
inline int auto_sprintf(WCHAR* buf, const WCHAR* format, ...) { va_list args; va_start(args, format); const int n = auto_vsprintf(buf, format, args); va_end(args); return n; }

template <typename CharT, basis::WritableBuffer<CharT> A>
int auto_vsnprintf_s(
	A& dst,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& v
) noexcept
{
	return cxx::_vsnprintf_s( dst, count, format, v );
}

template <typename CharT>
int auto_vsnprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& v
) noexcept
{
	return cxx::_vsnprintf_s( pBuffer, nBufferSize, count, format, v );
}

template <typename CharT, basis::WritableBuffer<CharT> A>
int auto_vsprintf_s(
	A& dst,
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& v
) noexcept
{
	return cxx::_vsprintf_s( dst, format, v );
}

template <typename CharT>
int auto_vsprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& v
) noexcept
{
	return cxx::_vsprintf_s( pBuffer, nBufferSize, format, v );
}

template <typename CharT, basis::WritableBuffer<CharT> A, typename... Params>
int auto_snprintf_s(
	A& dst,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		dst,
		count,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, typename... Params>
int auto_snprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	size_t count,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_snprintf_s(
		pBuffer,
		nBufferSize,
		count,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, basis::WritableBuffer<CharT> A, typename... Params>
int auto_sprintf_s(
	A& dst,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	return cxx::_sprintf_s(
		dst,
		format,
		std::as_const(params)...
	);
}

template <typename CharT, typename... Params>
int auto_sprintf_s(
	CharT* pBuffer,
	size_t nBufferSize,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメータ展開で転送する
	return cxx::_sprintf_s(
		pBuffer,
		nBufferSize,
		format,
		std::as_const(params)...
	);
}

/*!
 * @brief 文字列に書式付きデータを書き込みます。
 *
 * 生配列をSFilePathに置換できるよう用意したもの。
 *
 * セキュリティ上問題があるので、std::formatへの移行を検討してください。
 *
 * @return 書き込まれた文字数
 * @retval < 0 エラー発生
 */
template <basis::WritableBuffer<WCHAR> A, typename... Params>
int swprintf_s(
	A& dst,
	_In_z_ _Printf_format_string_ const WCHAR* format,
	const Params&... params
)
{
	return auto_sprintf_s(
		dst,
		format,
		std::as_const(params)...
	);
}

/*!
 * @brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
 * 	事前に確保したバッファに結果を書き込む高速バージョン
 *
 * @param[in, out] out フォーマットされたテキストを受け取る変数
 * @param[in] format フォーマット文字列
 * @param[in] argList 引数リスト
 * @returns 出力された文字数。NUL終端を含まない。
 * @retval >= 0 正常終了
 * @retval < 0 異常終了
 */
template <typename CharT>
inline int vstrprintf(
	std::basic_string<CharT>& out,
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& argList
)
{
	// 整形によって出力される文字数をカウント
	const int count = cxx::_vscprintf(format, argList);

	// 出力文字数が0未満ならエラー、戻り値は空。
	if (count <= 0) return count;

	// 出力先バッファを確保する
	out.resize(count);

	// 整形を実行する
	return cxx::_vsprintf_s(out.data(), out.size() + 1, format, argList);
}

/*!
 * @brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
 * 	動的にバッファを確保する簡易バージョン
 *
 * @param[in] format フォーマット文字列
 * @param[in] argList 引数リスト
 * @returns フォーマットされた文字列
 */
template <typename CharT>
inline std::basic_string<CharT> vstrprintf(
	_In_z_ _Printf_format_string_ const CharT* format,
	va_list& argList
)
{
	// 出力先バッファを用意する
	std::basic_string<CharT> out;

	// 整形を実行する
	const auto formatted = vstrprintf(out, format, argList);

	if (formatted <= 0) return {};

	// NUL終端する
	out.resize(formatted);

	return out;
}

/*!
 * @brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
 * 	事前に確保したバッファに結果を書き込む高速バージョン
 *
 * @param[in, out] out フォーマットされたテキストを受け取る変数
 * @param[in] format フォーマット文字列
 * @param[in, opt] params 引数リスト
 * @returns 出力された文字数。NUL終端を含まない。
 * @retval >= 0 正常終了
 * @retval < 0 異常終了
 */
template <typename CharT, typename... Params>
int strprintf(
	std::basic_string<CharT>& out,
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
)
{
	// パラメーターがない場合、コンパイルエラーにする
	static_assert(
		0 < sizeof...(params),
		"One or more paramaters should be passed"
	);

	// 整形によって出力される文字数をカウント
	const int count = cxx::_scprintf(format, std::as_const(params)...);

	// 出力文字数が0未満ならエラー、戻り値は空。
	if (count <= 0) return count;

	// 出力先バッファを確保する
	out.resize(count);

	// 整形を実行する
	return cxx::_sprintf_s(out.data(), out.size() + 1, format, std::as_const(params)...);
}

/*!
 * @brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
 * 	動的にバッファを確保する簡易バージョン
 *
 * @param[in] format フォーマット文字列
 * @param[in, opt] params 引数リスト
 * @returns フォーマットされた文字列
 */
template <typename CharT, typename... Params>
auto strprintf(
	_In_z_ _Printf_format_string_ const CharT* format,
	const Params&... params
) -> std::basic_string<CharT>
{
	// 出力先バッファを用意する
	std::basic_string<CharT> out;

	// 整形を実行する
	const auto formatted = strprintf(out, format, std::as_const(params)...);

	if (formatted <= 0) return {};

	// NUL終端する
	out.resize(formatted);

	return out;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       リテラル比較                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// リテラルとの文字列比較の際に、手打ちで文字数を入力するのは
// 手間が掛かる上に、保守性が損なわれるので、
// カプセル化された関数やマクロに処理を任せるのが望ましい。

//wcsncmpの文字数指定をliteralData2の大きさで取得してくれる版
template <size_t Size>
inline int wcsncmp_literal(const wchar_t* strData1, const wchar_t (&literalData2)[Size]) {
	return ::wcsncmp(strData1, literalData2, Size - 1 ); //※終端ヌルを含めないので、_countofからマイナス1する
}

//strncmpの文字数指定をliteralData2の大きさで取得してくれる版
template <size_t Size>
inline int strncmp_literal(const char* strData1, const char (&literalData2)[Size]) {
	return ::strncmp(strData1, literalData2, Size - 1 ); //※終端ヌルを含めないので、_countofからマイナス1する
}

//_wcsnicmpの文字数指定をliteralData2の大きさで取得してくれる版
template <size_t Size>
inline int wcsnicmp_literal(const wchar_t* strData1, const wchar_t (&literalData2)[Size]) {
	return ::_wcsnicmp(strData1, literalData2, Size - 1 ); //※終端ヌルを含めないので、_countofからマイナス1する
}

//_strnicmpの文字数指定をliteralData2の大きさで取得してくれる版
template <size_t Size>
inline int strnicmp_literal(const char* strData1, const char (&literalData2)[Size]) {
	return ::_strnicmp(strData1, literalData2, Size - 1 ); //※終端ヌルを含めないので、_countofからマイナス1する
}

#endif /* SAKURA_STRING_EX_87282FEB_4B23_4112_9C5A_419F43618705_H_ */
