//	$Id$
/*!	@file
	@brief 独自文字列比較関数

	@author MIK
	@date Jan. 11, 2002
	@date Feb. 02, 2002  内部処理を統一、全角アルファベット同一視に対応
	@date Apr. 07, 2005  MIK strstr系関数を追加
	$Revision$
*/
/*
	Copyright (C) 2002-2005, MIK
	Copyright (C) 2002, Moca

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
/*!
 * 大文字小文字を同一視する文字列比較、メモリ比較を独自に実装する。
 *
 *   stricmp, strnicmp, memicmp
 *
 * これはコンパイラと言語指定によって不正動作をしてしまうことを回避するための
 * ものです。
 * 日本語は SJIS です。
 *
 * stricmp, strnicmp, memicmp (および _ 付きのもの) を使用しているファイルの
 * 先頭に #include "my_icmp.h" を追加します。ただし、他のヘッダファイルよりも
 * 後になる場所に追加してください。
 *   →関数をマクロで定義し直すため。
 */



/*
 * ヘッダ
 */
#include "stdafx.h"
#include <stdio.h>
#include <limits.h>
#include "my_icmp.h"



/*!	大文字に変換する。
	@param c [in] 変換する文字コード

	@return 変換された文字コード
*/
SAKURA_CORE_API MY_INLINE int my_toupper( int c )
{
	if( c >= 'a' && c <= 'z' ) return c - ('a' - 'A');
	return c;
}



/*!	小文字に変換する。
	@param c [in] 変換する文字コード

	@return 変換された文字コード
*/
SAKURA_CORE_API MY_INLINE int my_tolower( int c )
{
	if( c >= 'A' && c <= 'Z' ) return c + ('a' - 'A');
	return c;
}



/*!	日本語１バイト目か調べる。
	@param c [in] 検査する文字コード

	@retval 1	漢字１バイト目である
	@retval 0	漢字１バイト目ではない
*/
SAKURA_CORE_API MY_INLINE int my_iskanji1( int c )
{
	if( (c >= 0x81 && c <= 0x9f) || (c >= 0xe0 && c <= 0xfc) ) return 1;
	return 0;
}



/*!	日本語２バイト目か調べる。
	@param c [in] 検査する文字コード

	@retval 1	漢字２バイト目である
	@retval 0	漢字２バイト目ではない
*/
SAKURA_CORE_API MY_INLINE int my_iskanji2( int c )
{
	if( (c >= 0x40 && c <= 0x7e) || (c >= 0x80 && c <= 0xfc) ) return 1;
	return 0;
}



#ifdef MY_ICMP_MBS
/*!	全角アルファベットの２文字目を大文字に変換する。
	@param c [in] 変換する文字コード

	@note
		0x8260 - 0x8279 : Ａ...Ｚ
		0x8281 - 0x829a : ａ...ｚ

	@return 変換された文字コード
*/
SAKURA_CORE_API MY_INLINE int my_mbtoupper2( int c )
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
SAKURA_CORE_API MY_INLINE int my_mbtolower2( int c )
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
SAKURA_CORE_API MY_INLINE int my_mbisalpha2( int c )
{
	if( (c >= 0x60 && c <= 0x79) || (c >= 0x81 && c <= 0x9a) ) return 1;
	return 0;
}
#endif  /* MY_ICMP_MBS */



/*!	大文字小文字を同一視する文字列長さ制限比較をする。
	@param s1   [in] 文字列１
	@param s2   [in] 文字列２
	@param n    [in] 文字長
	@param dcount  [in] ステップ値 (1=strnicmp,memicmp, 0=stricmp)
	@param flag [in] 文字列終端チェック (true=stricmp,strnicmp, false=memicmp)

	@retval 0	一致
	@date 2002.11.29 Moca 0以外の時の戻り値を，「元の値の差」から「大文字としたときの差」に変更
 */
SAKURA_CORE_API int __cdecl my_internal_icmp( const char *s1, const char *s2, unsigned int n, unsigned int dcount, bool flag )
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
//					c1_lo = my_mbtolower2( c1 );
//					c1_up = my_mbtoupper2( c1 );
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
//			c1_lo = my_tolower(c1);
//			c1_up = my_toupper(c1);
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
//					c2_lo = my_mbtolower2( c2 );
//					c2_up = my_mbtoupper2( c2 );
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
//			c2_lo = my_tolower(c2);
//			c2_up = my_toupper(c2);
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



/*!	大文字小文字を同一視するメモリ比較をする。
	@param m1 [in] データ１
	@param m2 [in] データ２
	@param n [in] データ長

	@retval 0	一致

	@note
	本来メモリ比較はバイナリデータを比較するものであり、比較対象データが
	日本語であろうがなかろうが関係ない。
	しかし、テキストデータにバイナリが含まれることがあるため memicmp で
	比較を行っている。
	ここでは日本語に対応した memicmp を実装する。
*/
SAKURA_CORE_API int __cdecl my_memicmp( const void *m1, const void *m2, unsigned int n )
{
	return my_internal_icmp( (const char*)m1, (const char*)m2, n, 1, false );
}



/*!	大文字小文字を同一視する文字列比較をする。
	@param s1 [in] 文字列１
	@param s2 [in] 文字列２

	@retval 0	一致
 */
SAKURA_CORE_API int __cdecl my_stricmp( const char *s1, const char *s2 )
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
SAKURA_CORE_API int __cdecl my_strnicmp( const char *s1, const char *s2, size_t n )
{
	return my_internal_icmp( s1, s2, (unsigned int)n, 1, true );
}

/*!
	strstr()の大文字小文字同一視版

	@note
	Windows APIにあるStrStrIはIE4が入っていないPCでは使用不可のため
	独自に作成

	@date 2005.04.07 MIK 新規作成
*/
const char* my_strstri( const char* s1, const char* s2 ){
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_strnicmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	strstr()の2byte code対応版

	@date 2005.04.07 MIK 新規作成
*/
const char* my_strstr( const char* s1, const char* s2 ){
	size_t n = strlen( s2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( strncmp( p1, s2, n ) == 0 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK 新規作成
*/
const char* my_strchri( const char* s1, int c2 ){
	int C2 = my_toupper( c2 );
	for( const char* p1 = s1; *p1; p1++ ){
		if( my_toupper( *p1 ) == C2 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}

/*!
	@date 2005.04.07 MIK 新規作成
*/
const char* my_strchr( const char* s1, int c2 ){
	for( const char* p1 = s1; *p1; p1++ ){
		if( *p1 == c2 ) return p1;
		if( my_iskanji1( *(const unsigned char*)p1 ) && *(p1+1) != 0 ) p1++;
	}
	return NULL;
}
/*[EOF]*/
