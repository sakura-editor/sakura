/*! @file */
/*
	Copyright (C) 2018-2021, Sakura Editor Organization

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
	_Tマクロ互換のテンプレート。
	ビルド種に関わらず、指定した型の文字定数を提供する。

	_T2(char,'A')
	_T2(wchar_t,'x')
	_T2(WCHAR,'/')
	のように使います。

	テンプレートてんこもりなので、コンパイルが重くなると思われます。
	インクルードは最小限に！

	2007.10.23 kobake 作成
*/

typedef char ACHAR;
typedef wchar_t WCHAR;

template <class CHAR_TYPE, int CHAR_VALUE>
CHAR_TYPE _TextTemplate();

//文字定義マクロ
#define DEFINE_T2(CHAR_VALUE) \
template<> ACHAR _TextTemplate<ACHAR,CHAR_VALUE>(){ return ATEXT(CHAR_VALUE); } \
template<> WCHAR _TextTemplate<WCHAR,CHAR_VALUE>(){ return LTEXT(CHAR_VALUE); }

//使用マクロ
#define _T2(CHAR_TYPE,CHAR_VALUE) _TextTemplate<CHAR_TYPE,CHAR_VALUE>()
