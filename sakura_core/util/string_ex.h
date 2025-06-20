﻿/*! @file */
/*
	Copyright (C) 2008, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STRING_EX_87282FEB_4B23_4112_9C5A_419F43618705_H_
#define SAKURA_STRING_EX_87282FEB_4B23_4112_9C5A_419F43618705_H_
#pragma once

#include <vadefs.h>
#include <string>
#include <string_view>
#include "basis/primitive.h"
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
inline wchar_t my_towupper2( wchar_t c ){ return my_towupper(c); }
inline wchar_t my_towlower2( wchar_t c ){ return my_towlower(c); }
int skr_towupper( int c );
int skr_towlower( int c );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           拡張・独自実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//文字数の上限付きコピー
LPWSTR wcscpyn(LPWSTR lpString1,LPCWSTR lpString2,int iMaxLength); //iMaxLengthは文字単位。

//	Apr. 03, 2003 genta
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count);

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
inline errno_t auto_strcpy_s(ACHAR* dst, size_t nDstCount, const ACHAR* src){ return strcpy_s(dst,nDstCount,src); }
inline errno_t auto_strcpy_s(WCHAR* dst, size_t nDstCount, const WCHAR* src){ return wcscpy_s(dst,nDstCount,src); }
inline ACHAR* auto_strncpy(ACHAR* dst,const ACHAR* src,size_t count){ return strncpy(dst,src,count); }
inline WCHAR* auto_strncpy(WCHAR* dst,const WCHAR* src,size_t count){ return wcsncpy(dst,src,count); }
inline ACHAR* auto_memset(ACHAR* dest, ACHAR c, size_t count){        memset (dest,c,count); return dest; }
inline WCHAR* auto_memset(WCHAR* dest, WCHAR c, size_t count){ return wmemset(dest,c,count);              }
inline ACHAR* auto_strcat(ACHAR* dst, const ACHAR* src){ return strcat(dst,src); }
inline WCHAR* auto_strcat(WCHAR* dst, const WCHAR* src){ return wcscat(dst,src); }
inline errno_t auto_strcat_s(ACHAR* dst, size_t nDstCount, const ACHAR* src){ return strcat_s(dst,nDstCount,src); }
inline errno_t auto_strcat_s(WCHAR* dst, size_t nDstCount, const WCHAR* src){ return wcscat_s(dst,nDstCount,src); }

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
inline size_t auto_strnlen(const ACHAR* str, size_t count){ return strnlen(str, count); }
inline size_t auto_strnlen(const WCHAR* str, size_t count){ return wcsnlen(str, count); }

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

inline int auto_vsprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v) { return ::_vsnprintf_s(buf, nBufCount, _TRUNCATE, format, v); }
inline int auto_vsprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v) { return ::_vsnwprintf_s(buf, nBufCount, _TRUNCATE, format, v); }
#define auto_snprintf_s(buf, nBufCount, format, ...)	::_snwprintf_s((buf), nBufCount, _TRUNCATE, (format), __VA_ARGS__)

std::wstring& eos(std::wstring& strOut, size_t cchOut);
std::string& eos(std::string& strOut, size_t cchOut);

int vstrprintf(std::wstring& strOut, const WCHAR* pszFormat, va_list& argList);
int vstrprintf(std::string& strOut, const CHAR* pszFormat, va_list& argList);
int strprintf(std::wstring& strOut, const WCHAR* pszFormat, ...);
int strprintf(std::string& strOut, const CHAR* pszFormat, ...);
std::wstring vstrprintf(const WCHAR* pszFormat, va_list& argList);
std::string vstrprintf(const CHAR* pszFormat, va_list& argList);
std::wstring strprintf(const WCHAR* pszFormat, ...);
std::string strprintf(const CHAR* pszFormat, ...);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      文字コード変換                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#include <vector>

//SJIS→UNICODE。終端にL'\0'を付けてくれる版。
size_t mbstowcs2(wchar_t* dst,const char* src,size_t dst_count);
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount);

//UNICODE→SJIS。終端に'\0'を付けてくれる版。
size_t wcstombs2(char* dst,const wchar_t* src,size_t dst_count);

//SJIS→UNICODE。
wchar_t*	mbstowcs_new(const char* pszSrc);								//戻り値はnew[]で確保して返す。使い終わったらdelete[]すること。
wchar_t*	mbstowcs_new(const char* pSrc, int nSrcLen, int* pnDstLen);		//戻り値はnew[]で確保して返す。使い終わったらdelete[]すること。
void		mbstowcs_vector(const char* src, std::vector<wchar_t>* ret);	//戻り値はvectorとして返す。
void		mbstowcs_vector(const char* pSrc, int nSrcLen, std::vector<wchar_t>* ret);	//戻り値はvectorとして返す。

//UNICODE→SJIS
char*	wcstombs_new(const wchar_t* src); //戻り値はnew[]で確保して返す。
char*	wcstombs_new(const wchar_t* pSrc,int nSrcLen); //戻り値はnew[]で確保して返す。
void	wcstombs_vector(const wchar_t* pSrc, std::vector<char>* ret); //戻り値はvectorとして返す。
void	wcstombs_vector(const wchar_t* pSrc, int nSrcLen, std::vector<char>* ret); //戻り値はvectorとして返す。

std::wstring u8stowcs(std::wstring& strOut, std::string_view strInput);
std::string wcstou8s(std::string& strOut, std::wstring_view strInput);
std::wstring u8stowcs(std::string_view strInput);
std::string wcstou8s(std::wstring_view strInput);

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
