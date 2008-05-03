#pragma once


// -- -- -- -- 論理型 -- -- -- -- //

// novice 2002/09/14
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
#define BOOL	int
#endif


// -- -- -- -- 定数 -- -- -- -- //

#ifndef NULL
#define NULL 0
#endif


// -- -- -- -- 文字 -- -- -- -- //

//char,wchar_t の変わりに、別名の ACHAR,WCHAR を使うと、ソース整形がしやすいケースがある。
typedef char ACHAR;


//TCHAR追加機能
//TCHARと逆の文字型をNOT_TCHARとして定義する
#ifdef _UNICODE
typedef char NOT_TCHAR;
#else
typedef wchar_t NOT_TCHAR;
#endif


//WIN_CHAR (WinAPIに渡すので、必ずTCHARでなければならないもの)
typedef TCHAR WIN_CHAR;
#define _WINT(A) _T(A)


//EDIT_CHAR
typedef wchar_t WChar;      //エディタで用いるテキスト管理データ型
typedef wchar_t EDIT_CHAR;
#define _EDITL(A) LTEXT(A)


//文字コード別、文字型
typedef unsigned char	uchar_t;		//  unsigned char の別名．
typedef unsigned short	uchar16_t;		//  UTF-16 用．
typedef unsigned long	uchar32_t;		//  UTF-32 用．
typedef long			wchar32_t;



// -- -- -- -- その他 -- -- -- -- //

typedef char KEYCODE;

//int互換
#ifdef USE_STRICT_INT
	#include "CLaxInteger.h"
	typedef CLaxInteger Int;
#else
	typedef int Int;
#endif

