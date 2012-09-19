/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2007, kobake

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

#include <windows.h>
#include "global.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
SAKURA_CORE_API void AssertError( LPCTSTR pszFile, long nLine, BOOL bIsError );
SAKURA_CORE_API void DebugOutA( LPCSTR lpFmt, ...);
#ifdef _UNICODE
#define DebugOut DebugOutW
#else
#define DebugOut DebugOutA
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake メッセージボックスの使用はデバッグ時に限らないので、「Debug～」という名前を廃止

//テキスト整形機能付きMessageBox
SAKURA_CORE_API int MessageBoxF( HWND, UINT, LPCTSTR, LPCTSTR, ... );


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 デバッグ用メッセージ出力                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
*/
#ifdef _DEBUG
	#define MYTRACE DebugOut
	#define MYTRACE_A DebugOutA
#endif
#ifndef _DEBUG
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
#define DBPRINT DebugOut
#else
#if (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
#define DBPRINT(...)
#else
// Not support C99 variable macro
inline void DBPRINT( ... ){}
#endif
#endif // _DEBUG

#define DEBUG_TRACE DBPRINT

#define MYASSERT AssertError


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                ユーザ用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//$$メモ：Debug.h以外の置き場所を考える

//デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF


//エラー：赤丸に「×」[OK]
#define ErrorBeep()     MessageBeep(MB_ICONSTOP)

//警告：三角に「！」[OK]
#define WarningBeep()   MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」[OK]
#define InfoBeep()      MessageBeep(MB_ICONINFORMATION)

//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
#define ConfirmBeep()   MessageBeep(MB_ICONQUESTION)




// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         フラグ等                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#ifdef _DEBUG
	//!	設定している場所はあるが，参照している場所がない変数
	SAKURA_CORE_API extern int gm_ProfileOutput;
#endif



///////////////////////////////////////////////////////////////////////
#endif /* _DEBUG_H_ */


/*[EOF]*/
