//2007.10.18 kobake 作成
/*
	Copyright (C) 2007, kobake

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
#ifndef SAKURA_STD_MACRO_A4AD5AD7_E307_4F40_A051_F4301FC8DA58_H_
#define SAKURA_STD_MACRO_A4AD5AD7_E307_4F40_A051_F4301FC8DA58_H_

#define SAFE_DELETE(p) { delete p; p=0; }


/*
	2007.10.18 kobake
	テンプレート式 min とか max とか。

	どっかの標準ヘッダに、同じようなものがあった気がするけど、
	NOMINMAX を定義するにしても、なんだか min とか max とかいう名前だと、
	テンプレートを呼んでるんだかマクロを呼んでるんだか訳分かんないので、
	明示的に「t_～」という名前を持つ関数を用意。
*/

template <class T>
inline T t_min(T t1,T t2)
{
	return t1<t2?t1:t2;
}

template <class T>
inline T t_max(T t1,T t2)
{
	return t1>t2?t1:t2;
}

template <class T>
T t_abs(T t)
{
	return t>=T(0)?t:T(-t);
}

template <class T>
T t_unit(T t)
{
	return
		t>T(0)?1:
		t<T(0)?-1:
		0;
}


/*
	2007.10.19 kobake
	_countofマクロ。_countofが使えない古いコンパイラ用。

	ただし、他の場所でテンプレートごりごり使っているので、
	どっちにしろ古い環境でビルドは通らない予感。
*/

#ifndef _countof
#define _countof(A) (sizeof(A)/sizeof(A[0]))
#endif

//sizeof
#define sizeof_raw(V)  sizeof(V)
#define sizeof_type(V) sizeof(V)


/*
	2007.10.19 kobake

	リテラル文字列種、明示指定マクロ
*/

//ビルド種に関係なく、UNICODE。
#define __LTEXT(A) L##A
#define LTEXT(A) __LTEXT(A)
#define LCHAR(A) __LTEXT(A)

//ビルド種に関係なく、ANSI。
#define ATEXT(A) A

#endif /* SAKURA_STD_MACRO_A4AD5AD7_E307_4F40_A051_F4301FC8DA58_H_ */
/*[EOF]*/
