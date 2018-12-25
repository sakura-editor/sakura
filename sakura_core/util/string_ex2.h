/*! @file */
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
class CNativeA;
class CNativeW;

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

int AddLastChar( TCHAR* pszPath, int nMaxLen, TCHAR c );/* 2003.06.24 Moca 最後の文字が指定された文字でないときは付加する */
int LimitStringLengthA( const ACHAR* pszData, int nDataLength, int nLimitLength, CNativeA& cmemDes );/* データを指定「文字数」以内に切り詰める */
int LimitStringLengthW( const WCHAR* pszData, int nDataLength, int nLimitLength, CNativeW& cmemDes );/* データを指定「文字数」以内に切り詰める */
#ifdef _UNICODE
#define LimitStringLengthT LimitStringLengthW
#else
#define LimitStringLengthT LimitStringLengthA
#endif

const char* GetNextLimitedLengthText( const char* pText, int nTextLen, int nLimitLen, int* pnLineLen, int* pnBgn );/* 指定長以下のテキストに切り分ける */
const char*    GetNextLine  ( const char* pData, int nDataLen, int* pnLineLen, int* pnBgn, CEol* pcEol); /* CR0LF0,CRLF,LF,CRで区切られる「行」を返す。改行コードは行長に加えない */
const wchar_t* GetNextLineW ( const wchar_t* pData, int nDataLen, int* pnLineLen, int* pnBgn, CEol* pcEol, bool bExtEol); // GetNextLineのwchar_t版
//wchar_t* GetNextLineWB( const wchar_t*, int, int*, int*, CEol* ); // GetNextLineのwchar_t版(ビックエンディアン用)  // 未使用
void GetLineColumn( const wchar_t* pLine, int* pnJumpToLine, int* pnJumpToColumn );

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

/*! @brief int2dec の第2引数の文字列出力先に必要十分なサイズ取得用
	符号付き整数の最小値の場合に必要な長さを返す
*/
template <typename T>
constexpr size_t int2dec_destBufferSufficientLength();

/*!
	符号付き32bit整数の最小値(-2147483648)の10進数文字列の文字数は
	終端0文字を含めて12文字。
*/
template <>
constexpr size_t int2dec_destBufferSufficientLength<int32_t>()
{
	return _countof(L"-2147483648");
}

/*!
	符号付き64bit整数の最小値(-9223372036854775808)の10進数文字列の
	文字数は終端0文字を含めて21文字
*/
template <>
constexpr size_t int2dec_destBufferSufficientLength<int64_t>()
{
	return _countof(L"-9223372036854775808");
}

/*! @brief 整数を10進数の文字列に変換
	参考にしたコード : https://stackoverflow.com/a/12386915/4699324

	文字列出力先の領域サイズは変換される文字列の出力に必要なバッファ長の考慮が必要
	int2dec_destBufferSufficientLength 関数で必要十分な要素数の取得が可能

	@return 変換後の文字数（終端0文字の分は含まない）
*/
template <typename T, typename ChT>
ptrdiff_t int2dec(
	T value,	//!< [in] 文字列化の素になる整数
	ChT* sp		//!< [out] 文字列出力先
)
{
	// 符号無し整数型は対応外
	static_assert(std::is_signed<T>::value, "T must be signed type.");

	// 一時領域
	ChT tmp[int2dec_destBufferSufficientLength<T>()];
	ChT *tp = tmp;

	uint8_t minAdjuster = (value == std::numeric_limits<T>::min()) ? 1 : 0;
	// abs 関数に符号付き整数型の最小値を指定した場合の動作が未定義な事への対策
	// 最小値だった場合は 1 加算する事で abs 関数の出力値が
	// 符号付き整数型の最大値になるように事前に補正
	value += minAdjuster;

	T v = abs(value);

	// 下位桁から変換する
	do {
		// decimal only
		*tp++ = (ChT)('0' + (v % 10));
		v /= 10;
	} while (v);

	// 最小値を指定された場合に abs 関数実行前に事前に補正しているので
	// 事後にここで最下位桁に 1 加算する事で辻褄を合わせる
	tmp[0] += minAdjuster;

	ptrdiff_t len = tp - tmp;

	// 負の場合の符号文字
	if (value < 0) {
		*sp++ = '-';
		++len;
	}

	// 下位桁から出力しているので文字列を逆転する
	while (tp > tmp) {
		--tp;
		*sp = *tp;
		++sp;
	}

	// ゼロ終端文字追加
	*sp = '\0';

	return len;
}

#endif /* SAKURA_STRING_EX2_AA243462_59E7_4F55_B206_FD9ED8836A09_H_ */
/*[EOF]*/
