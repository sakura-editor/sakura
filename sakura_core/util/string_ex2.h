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
#ifndef SAKURA_STRING_EX2_AA243462_59E7_4F55_B206_FD9ED8836A09_H_
#define SAKURA_STRING_EX2_AA243462_59E7_4F55_B206_FD9ED8836A09_H_

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

int AddLastChar( TCHAR*, int, TCHAR );/* 2003.06.24 Moca 最後の文字が指定された文字でないときは付加する */
int LimitStringLengthA( const ACHAR*, int, int, CNativeA& );/* データを指定「文字数」以内に切り詰める */
int LimitStringLengthW( const WCHAR*, int, int, CNativeW& );/* データを指定「文字数」以内に切り詰める */
#ifdef _UNICODE
#define LimitStringLengthT LimitStringLengthW
#else
#define LimitStringLengthT LimitStringLengthA
#endif

const char* GetNextLimitedLengthText( const char*, int, int, int*, int* );/* 指定長以下のテキストに切り分ける */
const char*    GetNextLine  ( const char*   , int, int*, int*, CEol* ); /* CR0LF0,CRLF,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
const wchar_t* GetNextLineW ( const wchar_t*, int, int*, int*, CEol*, bool ); // GetNextLineのwchar_t版
//wchar_t* GetNextLineWB( const wchar_t*, int, int*, int*, CEol* ); // GetNextLineのwchar_t版(ビックエンディアン用)  // 未使用
void GetLineColumn( const wchar_t*, int*, int* );


int cescape(const TCHAR* org, TCHAR* buf, TCHAR cesc, TCHAR cwith);


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

#endif /* SAKURA_STRING_EX2_AA243462_59E7_4F55_B206_FD9ED8836A09_H_ */
/*[EOF]*/
