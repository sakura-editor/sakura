//	$Id$
/*!	@file
	@brief 独自比較関数

	@author MIK
	@date Jan. 11, 2002
	$Revision$
*/
/*
	Copyright (C) 2002, MIK

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
#include <stdio.h>
#include "my_icmp.h"



/*!	大文字に変換する。
	@param c[in] 変換する文字コード

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
	なお、日本語1バイト目を見つけたら次の文字は必ず日本語2バイト目と認識
	するので、バイナリデータによっては比較誤認識の可能性もあることに注意
	すること。
*/
SAKURA_CORE_API int my_memicmp( const void *m1, const void *m2, unsigned int n )
{
	unsigned int	i;
	unsigned char	*p1, *p2;
	int	c1, c1_lo, c1_up;
	int	c2, c2_lo, c2_up;
	int	prev1, prev2;

	p1 = (unsigned char*)m1;
	p2 = (unsigned char*)m2;
	prev1 = prev2 = 0;

	/* 指定長だけ繰り返す */
	for(i = n; i > 0; i--)
	{
		/* 比較対象となる文字を取得する */
		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);

		/* 文字１の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev1 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev1 = 0;
			/* 実は日本語でない場合、変換範囲にないので問題ない */
		}
		else if( my_iskanji1(c1) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev1 = 1;
			/* 日本語の２バイト目が不正でも１バイト目は変換範囲にないので問題ない */
		}
		else{
			c1_lo = my_tolower(c1);
			c1_up = my_toupper(c1);
		}

		/* 文字２の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev2 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev2 = 0;
			/* 実は日本語でない場合、変換範囲にないので問題ない */
		}
		else if( my_iskanji1(c2) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev2 = 1;
			/* 日本語の２バイト目が不正でも１バイト目は変換範囲にないので問題ない */
		}
		else{
			c2_lo = my_tolower(c2);
			c2_up = my_toupper(c2);
		}

		/* 比較する */
		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* 戻り値は元の文字の差 */

		/* ポインタを進める */
		p1++;
		p2++;
	}

	return 0;
}



/*!	大文字小文字を同一視する文字列比較をする。
	@param s1 [in] 文字列１
	@param s2 [in] 文字列２

	@retval 0	一致
 */
SAKURA_CORE_API int my_stricmp( const char *s1, const char *s2 )
{
	unsigned char	*p1, *p2;
	int	c1, c1_lo, c1_up;
	int	c2, c2_lo, c2_up;
	int	prev1, prev2;

	p1 = (unsigned char*)s1;
	p2 = (unsigned char*)s2;
	prev1 = prev2 = 0;

	/* 文字列の終端まで繰り返す */
	while(1)
	{
		/* 比較対象となる文字を取得する */
		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);

		/* 文字列の終端に達したか調べる */
		if( ! c1 ){
			if( ! c2 ) return 0;
			return 0 - c2;
		}
		else if( ! c2 ){
			return c1;
		}

		/* 文字１の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev1 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev1 = 0;
			/* 実は日本語でない場合、変換範囲にないので問題ない */
		}
		else if( my_iskanji1(c1) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev1 = 1;
			/* 日本語の２バイト目が不正でも１バイト目は変換範囲にないので問題ない */
		}
		else{
			c1_lo = my_tolower(c1);
			c1_up = my_toupper(c1);
		}

		/* 文字２の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev2 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev2 = 0;
			/* 実は日本語でない場合、変換範囲にないので問題ない */
		}
		else if( my_iskanji1(c2) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev2 = 1;
			/* 日本語の２バイト目が不正でも１バイト目は変換範囲にないので問題ない */
		}
		else{
			c2_lo = my_tolower(c2);
			c2_up = my_toupper(c2);
		}

		/* 比較する */
		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* 戻り値は元の文字の差 */

		/* ポインタを進める */
		p1++;
		p2++;
	}
	/*NOTREACHED*/
}



/*!	大文字小文字を同一視する文字列長さ制限比較をする。
	@param s1 [in] 文字列１
	@param s2 [in] 文字列２
	@param n [in] 文字長

	@retval 0	一致
 */
SAKURA_CORE_API int my_strnicmp( const char *s1, const char *s2, size_t n )
{
	unsigned int	i;
	unsigned char	*p1, *p2;
	int	c1, c1_lo, c1_up;
	int	c2, c2_lo, c2_up;
	int	prev1, prev2;

	p1 = (unsigned char*)s1;
	p2 = (unsigned char*)s2;
	prev1 = prev2 = 0;

	/* 指定長だけ繰り返す */
	for(i = n; i > 0; i--)
	{
		/* 比較対象となる文字を取得する */
		c1 = c1_lo = c1_up = (int)((unsigned int)*p1);
		c2 = c2_lo = c2_up = (int)((unsigned int)*p2);

		/* 文字列の終端に達したか調べる */
		if( ! c1 ){
			if( ! c2 ) return 0;
			return 0 - c2;
		}
		else if( ! c2 ){
			return c1;
		}

		/* 文字１の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev1 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev1 = 0;
			/* 実は日本語でない場合、変換範囲にないので問題ない */
		}
		else if( my_iskanji1(c1) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev1 = 1;
			/* 日本語の２バイト目が不正でも１バイト目は変換範囲にないので問題ない */
		}
		else{
			c1_lo = my_tolower(c1);
			c1_up = my_toupper(c1);
		}

		/* 文字２の日本語チェックを行い比較用の大文字小文字をセットする */
		if( prev2 ){	/* 前の文字が日本語１バイト目 */
			/* 今回は日本語２バイト目なので変換しない */
			prev2 = 0;
			/* 実は日本語でない場合、変換範囲にないので問題ない */
		}
		else if( my_iskanji1(c2) ){
			/* 今回は日本語１バイト目なので変換しない */
			prev2 = 1;
			/* 日本語の２バイト目が不正でも１バイト目は変換範囲にないので問題ない */
		}
		else{
			c2_lo = my_tolower(c2);
			c2_up = my_toupper(c2);
		}

		/* 比較する */
		if( (c1_lo - c2_lo) && (c1_up - c2_up) ) return c1 - c2;	/* 戻り値は元の文字の差 */

		/* ポインタを進める */
		p1++;
		p2++;
	}

	return 0;
}



/*[EOF]*/
