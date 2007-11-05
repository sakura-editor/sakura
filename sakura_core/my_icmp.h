/*!	@file
	@brief 独自比較関数

	@author MIK
	@date Jan. 11, 2002
	@date Feb. 02, 2002  内部処理を統一、全角アルファベット同一視に対応
*/
/*
	Copyright (C) 2002, MIK
	Copyright (C) 2005, MIK, genta

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
/*
 * 大文字小文字を同一視する文字列比較、メモリ比較を独自に実装する。
 *   stricmp, strnicmp, memicmp
 * これはコンパイラと言語指定によって不正動作をしてしまうことを回避するための
 * ものです。
 * 日本語は SJIS です。
 *
 * MY_ICMP_MBS @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *  これを定義すると全角のアルファベットも大文字小文字の区別がなくなります。
 * @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
 *
 * stricmp, strnicmp, memicmp (および _ 付きのもの) を使用しているファイルの
 * 先頭に #include "my_icmp.h" を追加します。ただし、他のヘッダファイルよりも
 * 後になる場所に追加してください。
 *   →関数をマクロで定義し直すため。
 */
#ifndef _MY_ICMP_H_
#define _MY_ICMP_H_



/*
 * ヘッダ
 */
#include "global.h"
//#define SAKURA_CORE_API



/*
 * マクロ
 */
#define MY_ICMP_MBS  /* マルチバイト対応をします */
#undef  MY_ICMP_MBS



/* 関数を再定義します */
#define  memicmp(a,b,c)    auto_memicmp((a),(b),(c))
#define  stricmp(a,b)      my_stricmp((a),(b))
#define  strnicmp(a,b,c)   my_strnicmp((a),(b),(c))

#ifdef   _memicmp
#undef   _memicmp
#endif
#define  _memicmp(a,b,c)   auto_memicmp((a),(b),(c))

#ifdef   _stricmp
#undef   _stricmp
#endif
#define  _stricmp(a,b)     my_stricmp((a),(b))

#ifdef   _strnicmp
#undef   _strnicmp
#endif
#define  _strnicmp(a,b,c)  my_strnicmp((a),(b),(c))


#define  MY_INLINE
//#define  MY_INLINE  inline

// 2005.04.07 MIK strstr系関数追加
#ifdef strstri
#  undef strstri
#endif
#define strstri(a,b)        my_strstri((a),(b))
#ifdef strchri
#  undef strchri
#endif
#define strchri(a,b)        my_strchri((a),(b))



// 2005.04.07 MIK strstr系関数追加
//	Aug. 21, 2005 genta 実体には__cdeclが無いので，宣言からも削除
SAKURA_CORE_API const char* my_strstri( const char* s1, const char* s2 );
SAKURA_CORE_API const char* my_strstr( const char* s1, const char* s2 );
SAKURA_CORE_API const char* my_strchri( const char* s1, int c2 );
SAKURA_CORE_API const char* my_strchr( const char* s1, int c2 );

inline SAKURA_CORE_API char* my_strstri( char* s1, const char* s2 )
{
	return const_cast<char*>(my_strstri(s1, s2));
}

inline SAKURA_CORE_API char* my_strstr( char* s1, const char* s2 )
{
	return const_cast<char*>(my_strstr(s1, s2));
}

inline SAKURA_CORE_API char* my_strchri( char* s1, int c2 )
{
	return const_cast<char*>(my_strchri(s1, c2));
}

inline SAKURA_CORE_API char* my_strchr( char* s1, int c2 )
{
	return const_cast<char*>(my_strchr(s1, c2));
}

#endif  /* _MY_ICMP_H_ */

