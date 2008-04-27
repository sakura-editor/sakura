#pragma once

class CEol;

// Aug. 16, 2007 kobake
wchar_t *wcsncpy_ex(wchar_t *dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src, size_t src_count);
wchar_t *wcs_pushW(wchar_t *dst, size_t dst_count, const wchar_t* src);
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src, size_t src_count);
wchar_t *wcs_pushA(wchar_t *dst, size_t dst_count, const char* src);
#ifdef _UNICODE
#define wcs_pushT wcs_pushW
#else
#define wcs_pushT wcs_pushA
#endif

SAKURA_CORE_API int AddLastChar( TCHAR*, int, TCHAR );/* 2003.06.24 Moca 最後の文字が指定された文字でないときは付加する */
SAKURA_CORE_API int LimitStringLengthA( const ACHAR*, int, int, CNativeA& );/* データを指定「文字数」以内に切り詰める */
SAKURA_CORE_API int LimitStringLengthW( const WCHAR*, int, int, CNativeW& );/* データを指定「文字数」以内に切り詰める */
SAKURA_CORE_API int LimitStringLengthT( const TCHAR*, int, int, CNativeT& );/* データを指定「文字数」以内に切り詰める */
SAKURA_CORE_API const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* 指定長以下のテキストに切り分ける */
SAKURA_CORE_API const char*    GetNextLine  ( const char*   , int, int*, int*, CEol* ); /* CR0LF0,CRLF,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
SAKURA_CORE_API const wchar_t* GetNextLineW ( const wchar_t*, int, int*, int*, CEol* ); // GetNextLineのwchar_t版
SAKURA_CORE_API void GetLineColm( const wchar_t*, int*, int* );
SAKURA_CORE_API int IsNumber( const wchar_t*, int, int );/* 数値ならその長さを返す */	//@@@ 2001.02.17 by MIK


SAKURA_CORE_API int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);


/*!	&の二重化
	メニューに含まれる&を&&に置き換える
	@author genta
	@date 2002/01/30 cescapeに拡張し，
	@date 2004/06/19 genta Generic mapping
*/
inline void dupamp(const TCHAR* org, TCHAR* out)
{	cescape( org, out, _T('&'), _T('&') ); }


/*
	scanf的安全スキャン

	使用例:
		int a[3];
		scan_ints("1,23,4,5", "%d,%d,%d", a);
		//結果: a[0]=1, a[1]=23, a[2]=4 となる。
*/
int scan_ints(
	const wchar_t*	pszData,	//!< [in]  データ文字列
	const wchar_t*	pszFormat,	//!< [in]  データフォーマット
	int*			anBuf		//!< [out] 取得した数値 (要素数は最大32まで)
);

