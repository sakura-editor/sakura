/*! @file */
//2007.10.18 kobake 作成
/*
	Copyright (C) 2007, kobake
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#ifndef SAKURA_STD_MACRO_ED0953D9_582D_40D6_8190_3FFA9344819D_H_
#define SAKURA_STD_MACRO_ED0953D9_582D_40D6_8190_3FFA9344819D_H_
#pragma once

#define SAFE_DELETE(p) { delete p; p=nullptr; }

/*
	2007.10.18 kobake
	テンプレート式 min とか max とか。

	どっかの標準ヘッダーに、同じようなものがあった気がするけど、
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
#endif /* SAKURA_STD_MACRO_ED0953D9_582D_40D6_8190_3FFA9344819D_H_ */
