/*!	@file
	@brief 独自文字列比較関数

	@author MIK
	@date Jan. 11, 2002
	@date Feb. 02, 2002  内部処理を統一、全角アルファベット同一視に対応
	@date Apr. 07, 2005  MIK strstr系関数を追加
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











/*[EOF]*/
