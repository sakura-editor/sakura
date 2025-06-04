/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "string_ex.h"

#include <errno.h>
#include <stdarg.h>
#include <array>
#include <memory>
#include <stdexcept>

#include "charset/charcode.h"
#include "charset/codechecker.h"
#include "util/std_macro.h"
#include <limits.h>
#include <locale.h>

int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                           文字                              //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                       拡張・独自実装                        //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!	大文字小文字を同一視する文字列比較をする。
	@param s1 [in] 文字列１
	@param s2 [in] 文字列２

	@retval 0	一致
 */
int __cdecl my_stricmp( const char *s1, const char *s2 )
{
	/* チェックする文字数をuint最大に設定する */
	//return my_internal_icmp( s1, s2, (unsigned int)(~0), 0, true );
	return my_internal_icmp( s1, s2, UINT_MAX, 0, true );
}

/*!	大文字小文字を同一視する文字列長さ制限比較をする。
	@param s1 [in] 文字列１
	@param s2 [in] 文字列２
	@param n [in] 文字長

	@retval 0	一致
 */
int __cdecl my_strnicmp( const char *s1, const char *s2, size_t n )
{
	return my_internal_icmp( s1, s2, (unsigned int)n, 1, true );
}

LPWSTR wcscpyn(LPWSTR lpString1,LPCWSTR lpString2,int iMaxLength)
{
	size_t len2=wcslen(lpString2);
	if((int)len2>iMaxLength-1)len2=iMaxLength-1;
	wmemcpy(lpString1,lpString2,len2);
	lpString1[len2]=L'\0';
	return lpString1;
}

/*
	WCHAR と WCHAR または ACHAR の変換関数
*/

ACHAR* tcstostr( ACHAR* dest, const WCHAR* src, size_t count){
	WCHAR* pr = const_cast<WCHAR*>(src);
	ACHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<ACHAR>(*pr);
		++pw;
	}
	return pw;
}
WCHAR* tcstostr( WCHAR* dest, const WCHAR* src, size_t count){
	WCHAR* pr = const_cast<WCHAR*>(src);
	WCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<WCHAR>(*pr);
		++pw;
	}
	return pw;
}

WCHAR* strtotcs( WCHAR* dest, const ACHAR* src, size_t count )
{
	ACHAR* pr = const_cast<ACHAR*>(src);
	WCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<WCHAR>(*pr);
		++pw;
	}
	return pw;
}
WCHAR* strtotcs( WCHAR* dest, const WCHAR* src, size_t count )
{
	WCHAR* pr = const_cast<WCHAR*>(src);
	WCHAR* pw = dest;
	for( ; pr < src+count; ++pr ){
		*pw = static_cast<WCHAR>(*pr);
		++pw;
	}
	return pw;
}

/*! 文字数制限機能付きstrncpy

	コピー先のバッファサイズから溢れないようにstrncpyする。
	バッファが不足する場合には2バイト文字の切断もあり得る。
	末尾の\0は付与されないが、コピーはコピー先バッファサイズ-1までにしておく。

	@param dst [in] コピー先領域へのポインタ
	@param dst_count [in] コピー先領域のサイズ
	@param src [in] コピー元
	@param src_count [in] コピーする文字列の末尾

	@retval 実際にコピーされたコピー先領域の1つ後を指すポインタ

	@author genta
	@date 2003.04.03 genta
*/
char *strncpy_ex(char *dst, size_t dst_count, const char* src, size_t src_count)
{
	if( src_count >= dst_count ){
		src_count = dst_count - 1;
	}
	memcpy( dst, src, src_count );
	return dst + src_count;
}

const wchar_t* wcsistr( const wchar_t* s1, const wchar_t* s2 )
{
	size_t len2=wcslen(s2);
	const wchar_t* p=s1;
	const wchar_t* q=wcschr(s1,L'\0')-len2;
	while(p<=q){
		if(wmemicmp(p,s2,len2)==0)return p;
		p++;
	}
	return NULL;
}

const char* stristr(const char* s1, const char* s2)
{
	//$ 日本語考慮してないので、あんまり役に立たない版。stristr_jを使うのが望ましい。
	size_t len2=strlen(s2);
	const char* p=s1;
	const char* q=strchr(s1,L'\0')-len2;
	while(p<=q){
		if(amemicmp(p,s2,len2)==0)return p;
		p++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK    新規作成
	@date 2007.10.21 kobake 関数名変更: my_strchri→strichr_j
*/
const char* strichr_j( const char* s1, char c2 )
{
	if(c2==0)return ::strchr(s1,0); //文字列終端を探すためにc2に0を渡した場合も、正しく処理されるように。 2007.10.16 kobake

	int C2 = my_toupper( c2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_toupper( *p1 ) == C2 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK    新規作成
	@date 2007.10.21 kobake 関数名変更: my_strchr→strchr_j
*/
const char* strchr_j(const char* str, char c)
{
	if(c==0)return ::strchr(str,0); //文字列終端を探すためにcに0を渡した場合も、正しく処理されるように。 2007.10.16 kobake

	for( const char* p1 = str; *p1; p1++ ){
		if( *p1 == c ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()の2byte code対応版

	@date 2005.04.07 MIK 新規作成
	@date 2007.10.21 kobake 関数名変更: my_strstr→strstr_j
*/
const char* strstr_j(const char* s1, const char* s2)
{
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( strncmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()の大文字小文字同一視版

	@note
	Windows APIにあるStrStrIはIE4が入っていないPCでは使用不可のため
	独自に作成

	@date 2005.04.07 MIK    新規作成
	@date 2007.10.21 kobake 関数名変更: my_strstri→stristr_j
*/
const char* stristr_j( const char* s1, const char* s2 )
{
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_strnicmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@brief 文字列の終端位置を調整する
	@param[in, out] strOut	終端位置修正対象のインスタンス
	@param[in]  cchOut		終端位置
	@returns 終端位置を調整されたインスタンス。
 */
std::wstring& eos(std::wstring& strOut, size_t cchOut)
{
	if (strOut.empty() && cchOut < 8) {
		std::wstring buf(strOut.data());
		strOut.assign(buf.data(), cchOut);
	}
	else {
		strOut.assign(strOut.data(), cchOut);
	}

	return strOut;
}

/*!
	@brief 文字列の終端位置を調整する
	@param[in, out] strOut	終端位置修正対象のインスタンス
	@param[in]  cchOut		終端位置
	@returns 終端位置を調整されたインスタンス。
 */
std::string& eos(std::string& strOut, size_t cchOut)
{
	if (strOut.empty() && cchOut < 16) {
		std::string buf(strOut.data());
		strOut.assign(buf.data(), cchOut);
	}
	else {
		strOut.assign(strOut.data(), cchOut);
	}

	return strOut;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		事前に確保したバッファに結果を書き込む高速バージョン
	@param[in, out] strOut	フォーマットされたテキストを受け取る変数
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  argList		引数リスト
	@returns 出力された文字数。NUL終端を含まない。
	@retval >= 0 正常終了
	@retval < 0 異常終了
 */
int vstrprintf(std::wstring& strOut, const WCHAR* pszFormat, va_list& argList)
{
	// _vscwprintf() はフォーマットに必要な文字数を返す。
	const int cchOut = ::_vscwprintf(pszFormat, argList);
	if (cchOut <= 0) {
		strOut.clear();
		return cchOut;
	}

	// 必要なバッファを確保する
	if (const size_t required = cchOut + 1;
		strOut.capacity() <= required)
	{
		strOut.resize(required, L'0');
	}

	// フォーマットする
	::vswprintf_s(strOut.data(), strOut.capacity(), pszFormat, argList);

	// NUL終端する
	eos(strOut, cchOut);

	return cchOut;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		事前に確保したバッファに結果を書き込む高速バージョン
	@param[in, out] strOut	フォーマットされたテキストを受け取る変数
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  argList		引数リスト
	@returns 出力された文字数。NUL終端を含まない。
	@retval >= 0 正常終了
	@retval < 0 異常終了
 */
int vstrprintf(std::string& strOut, const CHAR* pszFormat, va_list& argList)
{
	// _vscwprintf() はフォーマットに必要な文字数を返す。
	const int cchOut = ::_vscprintf(pszFormat, argList);
	if (cchOut <= 0) {
		strOut.clear();
		return cchOut;
	}

	// 必要なバッファを確保する
	if (const size_t required = cchOut + 1;
		strOut.capacity() <= required)
	{
		strOut.resize(required, L'0');
	}

	// フォーマットする
	::vsprintf_s(strOut.data(), strOut.capacity(), pszFormat, argList);

	// NUL終端する
	eos(strOut, cchOut);

	return cchOut;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		事前に確保したバッファに結果を書き込む高速バージョン
	@param[in, out] strOut	フォーマットされたテキストを受け取る変数
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  argList		引数リスト
	@returns 出力された文字数。NUL終端を含まない。
	@retval >= 0 正常終了
	@retval < 0 異常終了
 */
int strprintf(std::wstring& strOut, const WCHAR* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	const auto nRet = vstrprintf(strOut, pszFormat, argList);

	va_end(argList);

	return nRet;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		事前に確保したバッファに結果を書き込む高速バージョン
	@param[in, out] strOut	フォーマットされたテキストを受け取る変数
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  argList		引数リスト
	@returns 出力された文字数。NUL終端を含まない。
	@retval >= 0 正常終了
	@retval < 0 異常終了
 */
int strprintf(std::string& strOut, const CHAR* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	const auto nRet = vstrprintf(strOut, pszFormat, argList);

	va_end(argList);

	return nRet;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		動的にバッファを確保する簡易バージョン
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  ...			引数リスト
	@returns フォーマットされた文字列
 */
std::wstring vstrprintf(const WCHAR* pszFormat, va_list& argList)
{
	// 出力バッファを確保する
	std::wstring strOut;

	const auto nRet = vstrprintf(strOut, pszFormat, argList);
	assert(nRet >= 0);

	return strOut;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		動的にバッファを確保する簡易バージョン
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  ...			引数リスト
	@returns フォーマットされた文字列
 */
std::string vstrprintf(const CHAR* pszFormat, va_list& argList)
{
	// 出力バッファを確保する
	std::string strOut;

	const int nRet = vstrprintf(strOut, pszFormat, argList);
	assert(nRet >= 0);

	return strOut;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		動的にバッファを確保する簡易バージョン
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  ...			引数リスト
	@returns フォーマットされた文字列
 */
std::wstring strprintf(const WCHAR* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	const auto strOut = vstrprintf(pszFormat, argList);

	va_end(argList);

	return strOut;
}

/*!
	@brief C-Styleのフォーマット文字列を使ってデータを文字列化する。
		動的にバッファを確保する簡易バージョン
	@param[in]  pszFormat	フォーマット文字列
	@param[in]  ...			引数リスト
	@returns フォーマットされた文字列
 */
std::string strprintf(const CHAR* pszFormat, ...)
{
	va_list argList;
	va_start(argList, pszFormat);

	const auto strOut = vstrprintf(pszFormat, argList);

	va_end(argList);

	return strOut;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                      文字コード変換                         //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//SJIS→UNICODE。終端にL'\0'を付けてくれる版。
size_t mbstowcs2(wchar_t* dst,const char* src,size_t dst_count)
{
	size_t ret=::mbstowcs(dst,src,dst_count-1);
	dst[ret]=L'\0';
	return ret;
}
size_t mbstowcs2(wchar_t* pDst, int nDstCount, const char* pSrc, int nSrcCount)
{
	int ret=MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcCount,
		pDst,
		nDstCount-1
	);
	pDst[ret]=L'\0';
	return (size_t)ret;
}

//UNICODE→SJIS。終端に'\0'を付けてくれる版。
size_t wcstombs2(char* dst,const wchar_t* src,size_t dst_count)
{
	size_t ret=::wcstombs(dst,src,dst_count-1);
	dst[ret]='\0';
	return ret;
}

//SJIS→UNICODE。戻り値はnew[]で確保して返す。
wchar_t* mbstowcs_new(const char* src)
{
	size_t new_length=mbstowcs(NULL,src,0);
	wchar_t* ret=new wchar_t[new_length+1];
	mbstowcs(ret,src,new_length);
	ret[new_length]=L'\0';
	return ret;
}
wchar_t* mbstowcs_new(const char* pSrc, int nSrcLen, int* pnDstLen)
{
	//必要な領域サイズ
	int nNewLength = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		NULL,
		0
	);
	
	//確保
	wchar_t* pNew = new wchar_t[nNewLength+1];

	//変換
	nNewLength = MultiByteToWideChar(
		CP_SJIS,				// 2008/5/12 Uchi
		0,
		pSrc,
		nSrcLen,
		pNew,
		nNewLength
	);
	pNew[nNewLength] = L'\0';
	if( pnDstLen ){
		*pnDstLen = nNewLength;
	}
	return pNew;
}

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                          メモリ                             //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

int wmemicmp(const WCHAR* p1,const WCHAR* p2,size_t count)
{
	for(size_t i=0;i<count;i++){
		int n=skr_towlower(*p1++)-skr_towlower(*p2++);	//非ASCIIも変換
		if(n!=0)return n;
	}
	return 0;
}

int wmemicmp(const WCHAR* p1,const WCHAR* p2)
{
	return wmemicmp(p1,p2, t_max(wcslen(p1), wcslen(p2)));
}

int wmemicmp_ascii(const WCHAR* p1,const WCHAR* p2,size_t count)
{
	for(size_t i=0;i<count;i++){
		int n=my_towlower(*p1++)-my_towlower(*p2++);	//ASCIIのみ変換（高速）
		if(n!=0)return n;
	}
	return 0;
}

/*!
	空白を含むファイル名を考慮したトークンの分割
	
	先頭にある連続した区切り文字は無視する．
	
	@return トークン

	@date 2004.02.15 みく   最適化
	@date 2007.10.21 kobake テンプレート化
*/
//$ いちいち手間かかる。。
namespace{
	template <class T> struct Charset{};
	template <> struct Charset<ACHAR>{ static const ACHAR QUOT= '"'; };
	template <> struct Charset<WCHAR>{ static const WCHAR QUOT=L'"'; };
}
template <class CHAR_TYPE>
CHAR_TYPE* my_strtok(
	CHAR_TYPE*			pBuffer,	//[in] 文字列バッファ(終端があること)
	int					nLen,		//[in] 文字列の長さ
	int*				pnOffset,	//[in,out] オフセット
	const CHAR_TYPE*	pDelimiter	//[in] 区切り文字
)
{
	int i = *pnOffset;
	CHAR_TYPE* p;

	do {
		bool bFlag = false;	//ダブルコーテーションの中か？
		if( i >= nLen ) return NULL;
		p = &pBuffer[i];
		for( ; i < nLen; i++ )
		{
			if( pBuffer[i] == Charset<CHAR_TYPE>::QUOT ) bFlag = ! bFlag;
			if( ! bFlag )
			{
				if( auto_strchr( pDelimiter, pBuffer[i] ) )
				{
					pBuffer[i++] = L'\0';
					break;
				}
			}
		}
		*pnOffset = i;
	} while( ! *p );	//空のトークンなら次を探す
	return p;
}
//インスタンス化
template ACHAR* my_strtok(ACHAR*,int,int*,const ACHAR*);
template WCHAR* my_strtok(WCHAR*,int,int*,const WCHAR*);

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         実装補助                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

#ifdef MY_ICMP_MBS
int my_mbtoupper2( int c );
int my_mbtolower2( int c );
int my_mbisalpha2( int c );
#endif  /* MY_ICMP_MBS */

#ifdef MY_ICMP_MBS
/*!	全角アルファベットの２文字目を大文字に変換する。
	@param c [in] 変換する文字コード

	@note
		0x8260 - 0x8279 : Ａ...Ｚ
		0x8281 - 0x829a : ａ...ｚ

	@return 変換された文字コード
*/
int my_mbtoupper2( int c )
{
	if( c >= 0x81 && c <= 0x9a ) return c - (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */

#ifdef MY_ICMP_MBS
/*!	全角アルファベットの２文字目を小文字に変換する。
	@param c [in] 変換する文字コード

	@return 変換された文字コード
*/
int my_mbtolower2( int c )
{
	if( c >= 0x60 && c <= 0x79 ) return c + (0x81 - 0x60);
	return c;
}
#endif  /* MY_ICMP_MBS */

#ifdef MY_ICMP_MBS
/*!	全角アルファベットの２文字目か調べる。
	@param c [in] 検査する文字コード

	@retval 1	全角アルファベット２バイト目である
	@retval 0	ちがう
*/
int my_mbisalpha2( int c )
{
	if( (c >= 0x60 && c <= 0x79) || (c >= 0x81 && c <= 0x9a) ) return 1;
	return 0;
}
#endif  /* MY_ICMP_MBS */

/*!	大文字小文字を同一視する文字列長さ制限比較をする。
	@param s1   [in] 文字列１
	@param s2   [in] 文字列２
	@param n    [in] 文字長
	@param dcount  [in] ステップ値 (1=_strnicmp,_memicmp, 0=stricmp)
	@param flag [in] 文字列終端チェック (true=stricmp,_strnicmp, false=_memicmp)

	@retval 0	一致
	@date 2002.11.29 Moca 0以外の時の戻り値を，「元の値の差」から「大文字としたときの差」に変更
 */
int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag )
{
	unsigned int	i;
	unsigned char	*p1, *p2;
//	2002.11.29 Moca 元の値を保持する必要がなくなったため *_lo, *_upを削除
//	int	c1, c1_lo, c1_up;
//	int	c2, c2_lo, c2_up;
	int 	c1, c2;
	bool	prev1, prev2; /* 前の文字が SJISの１バイト目か */
#ifdef MY_ICMP_MBS
	bool	mba1, mba2;
#endif  /* MY_ICMP_MBS */

	p1 = (unsigned char*)s1;
	p2 = (unsigned char*)s2;
	prev1 = prev2 = false;
#ifdef MY_ICMP_MBS
	mba1 = mba2 = false;
#endif  /* MY_ICMP_MBS */

	/* 指定長だけ繰り返す */
	for(i = n; i > 0; i -= dcount)
	{
		/* 比較対象となる文字を取得する */
//		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
//		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);
		c1 = (int)((unsigned int)*p1);
		c2 = (int)((unsigned int)*p2);

		/* 2002.11.29 Moca 文字列の終端に達したか調べる部分 は後方へ移動 */

		/* 文字１の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev1 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev1 = false;
#ifdef MY_ICMP_MBS
			/* 全角文字のアルファベット */
			if( mba1 ){
				mba1 = false;
				if( my_mbisalpha2( c1 ) ){
					c1 = my_mbtoupper2( c1 );
				}
			}
#endif  /* MY_ICMP_MBS */
		}
		else if( my_iskanji1(c1) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev1 = true;
#ifdef MY_ICMP_MBS
			if( c1 == 0x82 ) mba1 = true;
#endif  /* MY_ICMP_MBS */
		}
		else{
			c1 = my_toupper(c1);
		}

		/* 文字２の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev2 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev2 = false;
#ifdef MY_ICMP_MBS
			/* 全角文字のアルファベット */
			if( mba2 ){
				mba2 = false;
				if( my_mbisalpha2( c2 ) ){
					c2 = my_mbtoupper2( c2 );
				}
			}
#endif  /* MY_ICMP_MBS */
		}
		else if( my_iskanji1(c2) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev2 = true;
#ifdef MY_ICMP_MBS
			if( c2 == 0x82 ) mba2 = true;
#endif  /* MY_ICMP_MBS */
		}
		else{
			c2 = my_toupper(c2);
		}

		/* 比較する */
//		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* 戻り値は元の文字の差 */
		if( c1 - c2 ) return c1 - c2;	/* 戻り値は大文字に変換した文字の差 */

		/* 2002.11.29 Moca 戻り値を変更したことにより，小文字→大文字変換の後に移動
		   片方だけ NULL文字 の場合は上の比較した時点で return するためその処理は不要 */
		if( flag ){
			/* 文字列の終端に達したか調べる */
			if( ! c1 ) return 0;
		}
		/* ポインタを進める */
		p1++;
		p2++;
	}

	return 0;
}

// skr_towupper() / skr_tolower()
//
// 2010.09.28 ryoji
// BugReport/64: towupper(c) によって U+00e0-U+00fc と U+0020 が同一視される問題の対策
// VC のランタイムは c < 256 の条件ではなぜか locale に対応した "ANSI 系の" 変換テーブル引きを行っている模様
// （Unicode 系変換関数なのに locale が "Japanese" だと c < 256 の範囲では SJIS 用らしき変換テーブルが使われる）
// それでは都合が悪いので c < 256 範囲の変換に "English"(Windows-1252) locale を利用する。
//   ・Unicode の最初の 256 個の符号位置は Windows-1252 の親戚の ISO-8859-1 由来。
//   ・相違は 0x80-0x9F の区間で、Windows-1252 では図形文字、ISO-8859-1(Unicode) では制御文字。
// ※ ランタイムの towupper(c)/tolower(c) が将来期待する動作になったとしてもこの方法を使い続けて問題無いはず
int skr_towupper( int c )
{
#ifdef _MSC_VER
	static wchar_t szMap[256];	// c < 256 用の変換テーブル
	static bool bInit = false;
	if( !bInit ){
		int i;
		_locale_t locale = _create_locale( LC_CTYPE, "English" );
		for( i = 0; i < 0x80; i++ ) szMap[i] = (wchar_t)my_towupper( i );	// 自前で変換
		for( ; i < 0xA0; i++ ) szMap[i] = (wchar_t)i;						// 無変換（制御コード部）
		for( ; i < 255; i++ ) szMap[i] = _towupper_l( (wchar_t)i, locale );	// "English"localeで変換
		szMap[255] = 0x0178;	// Windows-1252 だと 0x9f(制御文字域) にマップしてしまうので
		_free_locale( locale );
		bInit = true;
	}

	if( c < 256 ) return szMap[c];
#endif
	return towupper( (wchar_t)c );
}

int skr_towlower( int c )
{
#ifdef _MSC_VER
	static wchar_t szMap[256];	// c < 256 用の変換テーブル
	static bool bInit = false;
	if( !bInit ){
		int i;
		_locale_t locale = _create_locale( LC_CTYPE, "English" );
		for( i = 0; i < 0x80; i++ ) szMap[i] = (wchar_t)my_towlower( i );	// 自前で変換
		for( ; i < 0xA0; i++ ) szMap[i] = (wchar_t)i;						// 無変換（制御コード部）
		for( ; i < 256; i++ ) szMap[i] = _towlower_l( (wchar_t)i, locale );	// "English"localeで変換
		_free_locale( locale );
		bInit = true;
	}

	if( c < 256 ) return szMap[c];
#endif
	return towlower( (wchar_t)c );
}
