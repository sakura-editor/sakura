/*
	Copyright (C) 2008, kobake

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
*/
#ifndef SAKURA_STRING_EX_29EB1DD7_7259_4D6C_A651_B9174E5C3D3C9_H_
#define SAKURA_STRING_EX_29EB1DD7_7259_4D6C_A651_B9174E5C3D3C9_H_

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

#include "util/tchar_printf.h"

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
inline int amemicmp(const ACHAR* p1, const ACHAR* p2, size_t count){ return ::memicmp(p1,p2,count); }
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
#ifdef _UNICODE
#define _tcs_toupper skr_towupper
#define _tcs_tolower skr_towlower
#else
#define _tcs_toupper my_toupper
#define _tcs_tolower my_tolower
#endif

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
#ifdef _UNICODE
#define _tcsistr wcsistr
#else
#define _tcsistr stristr
#endif

//大文字小文字を区別せずに文字列を検索（日本語対応版）
const char* strchr_j(const char* s1, char c);				//!< strchr の日本語対応版。
const char* strichr_j( const char* s1, char c );			//!< strchr の大文字小文字同一視＆日本語対応版。
const char* strstr_j(const char* s1, const char* s2);		//!< strstr の日本語対応版。
const char* stristr_j( const char* s1, const char* s2 );	//!< strstr の大文字小文字同一視＆日本語対応版。
inline char* strchr_j ( char* s1, char c         ){ return const_cast<char*>(strchr_j ((const char*)s1, c )); }
inline char* strichr_j( char* s1, char c         ){ return const_cast<char*>(strichr_j((const char*)s1, c )); }
inline char* strstr_j ( char* s1, const char* s2 ){ return const_cast<char*>(strstr_j ((const char*)s1, s2)); }
inline char* stristr_j( char* s1, const char* s2 ){ return const_cast<char*>(stristr_j((const char*)s1, s2)); }
#ifdef _UNICODE
#define _tcsistr_j wcsistr
#else
#define _tcsistr_j stristr_j
#endif

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
//                           互換                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// VS2005以降の安全版文字列関数
#if (defined(_MSC_VER) && _MSC_VER<1400) \
	|| (defined(__MINGW32__) && defined(MINGW_HAS_SECURE_API) && MINGW_HAS_SECURE_API) //VS2005より前なら
	typedef int errno_t;
#define _TRUNCATE ((size_t)-1)
	errno_t strcpy_s(char *dest, size_t num, const char *src);
	errno_t wcscpy_s(wchar_t *dest, size_t num, const wchar_t *src);
	errno_t strncpy_s(char *dest, size_t num, const char *src, size_t count);
	errno_t wcsncpy_s(wchar_t *dest, size_t num, const wchar_t *src, size_t count);
	errno_t strcat_s(char *dest, size_t num, const char *src);
	errno_t wcscat_s(wchar_t *dest, size_t num, const wchar_t *src);

	int vsprintf_s(char *buf, size_t num, const char *fmt, va_list vaarg);
	int vswprintf_s(wchar_t *buf, size_t num, const wchar_t *fmt, va_list vaarg);
	int vsnprintf_s(char *buf, size_t num, size_t count, const char *fmt, va_list vaarg);
	int _vsnwprintf_s(wchar_t *buf, size_t num, size_t count, const wchar_t *fmt, va_list vaarg);

	size_t strnlen(const char *str, size_t num);
	size_t wcsnlen(const wchar_t *str, size_t num);
#ifdef _UNICODE
#define _tcscpy_s wcscpy_s
#define _tcsncpy_s wcsncpy_s
#define _tcscat_s wcscat_s
#define _tcsnlen wcsnlen
#define _tcsncicmp _wcsnicmp
#define _ttempnam _wtempnam
#define _tWinMain wWinMain
#else
#define _tcscpy_s strcpy_s
#define _tcsncpy_s strncpy_s
#define _tcscat_s strcat_s
#define _tcsnlen strnlen
#define _tcsncicmp _strnicmp
#define _ttempnam tempnam
#define _tWinMain WinMain
#endif
#endif

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
ACHAR* tcstostr( ACHAR* dest, const TCHAR* src, size_t count );
WCHAR* tcstostr( WCHAR* dest, const TCHAR* src, size_t count );
TCHAR* strtotcs( TCHAR* dest, const ACHAR* src, size_t count );
TCHAR* strtotcs( TCHAR* dest, const WCHAR* src, size_t count );

//印字系
#if defined(_MSC_VER) && _MSC_VER>=1400
#define auto_snprintf_s(buf, count, format, ...) tchar_sprintf_s((buf), count, (format), __VA_ARGS__)
#define auto_sprintf(buf, format, ...)           tchar_sprintf((buf), (format), __VA_ARGS__)
#define auto_sprintf_s(buf, nBufCount, format, ...) tchar_snprintf_s((buf), nBufCount, (format), __VA_ARGS__)
#else
inline int auto_snprintf_s(ACHAR* buf, size_t count, const ACHAR* format, ...)   { va_list v; va_start(v,format); int ret=tchar_vsnprintf_s(buf,count,format,v); va_end(v); return ret; }
inline int auto_snprintf_s(WCHAR* buf, size_t count, const WCHAR* format, ...)   { va_list v; va_start(v,format); int ret=tchar_vsnprintf_s(buf,count,format,v); va_end(v); return ret; }
inline int auto_sprintf(ACHAR* buf, const ACHAR* format, ...)                    { va_list v; va_start(v,format); int ret=tchar_vsprintf(buf,format,v); va_end(v); return ret; }
inline int auto_sprintf(WCHAR* buf, const WCHAR* format, ...)                    { va_list v; va_start(v,format); int ret=tchar_vsprintf(buf,format,v); va_end(v); return ret; }
inline int auto_sprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, ...){ va_list v; va_start(v,format); int ret=tchar_vsprintf_s(buf,nBufCount,format,v); va_end(v); return ret; }
inline int auto_sprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, ...){ va_list v; va_start(v,format); int ret=tchar_vsprintf_s(buf,nBufCount,format,v); va_end(v); return ret; }
#endif

inline int auto_vsprintf(ACHAR* buf, const ACHAR* format, va_list& v){ return tchar_vsprintf(buf,format,v); }
inline int auto_vsprintf(WCHAR* buf, const WCHAR* format, va_list& v){ return tchar_vsprintf(buf,format,v); }
inline int auto_vsprintf_s(ACHAR* buf, size_t nBufCount, const ACHAR* format, va_list& v){ return tchar_vsprintf_s(buf, nBufCount, format, v); }
inline int auto_vsprintf_s(WCHAR* buf, size_t nBufCount, const WCHAR* format, va_list& v){ return tchar_vsprintf_s(buf, nBufCount, format, v); }


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

//TCHAR
size_t _tcstowcs(WCHAR* wszDst, const TCHAR* tszSrc, size_t nDstCount);
size_t _tcstombs(CHAR*  szDst,  const TCHAR* tszSrc, size_t nDstCount);
size_t _wcstotcs(TCHAR* tszDst, const WCHAR* wszSrc, size_t nDstCount);
size_t _mbstotcs(TCHAR* tszDst, const CHAR*  szSrc,  size_t nDstCount);
int _tctomb(const TCHAR* p,ACHAR* mb);
int _tctowc(const TCHAR* p,WCHAR* wc);





// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       リテラル比較                          //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
// リテラルとの文字列比較の際に、手打ちで文字数を入力するのは
// 手間が掛かる上に、保守性が損なわれるので、
// カプセル化された関数やマクロに処理を任せるのが望ましい。

//wcsncmpの文字数指定をszData2からwcslenで取得してくれる版
inline int wcsncmp_auto(const wchar_t* strData1, const wchar_t* szData2)
{
	return wcsncmp(strData1,szData2,wcslen(szData2));
}

//wcsncmpの文字数指定をliteralData2の大きさで取得してくれる版
#define wcsncmp_literal(strData1, literalData2) \
	::wcsncmp(strData1, literalData2, _countof(literalData2) - 1 ) //※終端ヌルを含めないので、_countofからマイナス1する

//strncmpの文字数指定をliteralData2の大きさで取得してくれる版
#define strncmp_literal(strData1, literalData2) \
	::strncmp(strData1, literalData2, _countof(literalData2) - 1 ) //※終端ヌルを含めないので、_countofからマイナス1する

//TCHAR
#ifdef _UNICODE
	#define _tcsncmp_literal wcsncmp_literal
#else
	#define _tcsncmp_literal strncmp_literal
#endif

#endif /* SAKURA_STRING_EX_29EB1DD7_7259_4D6C_A651_B9174E5C3D3C9_H_ */
/*[EOF]*/
