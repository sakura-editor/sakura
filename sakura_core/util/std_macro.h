//2007.10.18 kobake 作成

#pragma once


#define SAFE_DELETE(p) do{ if(p){ delete p; p=0; } }while(0)


/*
	2007.10.18 kobake
	テンプレート式 min とか max とか。

	どっかの標準ヘッダに、同じようなものがあった気がするけど、
	NOMINMAX を定義するにしても、なんだか min とか max とかいう名前だと、
	テンプレートを呼んでるんだかマクロを呼んでるんだか訳分かんないので、
	明示的に「t_～」という名前を持つ関数を用意。
*/

template <class T>
T t_min(T t1,T t2)
{
	return t1<t2?t1:t2;
}

template <class T>
T t_max(T t1,T t2)
{
	return t1>t2?t1:t2;
}

template <class T>
T t_abs(T t)
{
	return t>=T(0)?t:T(-t);
}

template <class T>
void t_swap(T& t1, T& t2)
{
	T tmp = t1;
	t1 = t2;
	t2 = tmp;
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


