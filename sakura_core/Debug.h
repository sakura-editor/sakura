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
#if defined(_DEBUG) || defined(USE_RELPRINT)
void DebugOutW( LPCWSTR lpFmt, ...);
void DebugOutA( LPCSTR lpFmt, ...);
#endif	// _DEBUG || USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 デバッグ用メッセージ出力                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*
	MYTRACEはリリースモードではコンパイルエラーとなるようにしてあるので，
	MYTRACEを使う場合には必ず#ifdef _DEBUG ～ #endif で囲む必要がある．
*/
#ifdef _DEBUG
	#ifdef _UNICODE
	#define MYTRACE DebugOutW
	#else
	#define MYTRACE DebugOutA
	#endif
#else
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
	#ifdef _UNICODE
	#define DEBUG_TRACE DebugOutW
	#else
	#define DEBUG_TRACE DebugOutA
	#endif
#elif (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
	#define DEBUG_TRACE(...)
#else
	// Not support C99 variable macro
	inline void DEBUG_TRACE( ... ){}
#endif

//RELEASE版でも出力する版 (RELEASEでのみ発生するバグを監視する目的)
#ifdef USE_RELPRINT
	#ifdef _UNICODE
	#define RELPRINT DebugOutW
	#else
	#define RELPRINT DebugOutA
	#endif
#else
	#define RELPRINT   Do_not_define_USE_RELPRINT
#endif	// USE_RELPRINT

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake メッセージボックスの使用はデバッグ時に限らないので、「Debug～」という名前を廃止

//テキスト整形機能付きMessageBox
int VMessageBoxF( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, va_list& v );
int MessageBoxF ( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, ... );

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                ユーザ用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//$$メモ：Debug.h以外の置き場所を考える

//デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF

//一般の警告音
#define DefaultBeep()   MessageBeep(MB_OK)

//エラー：赤丸に「×」[OK]
int ErrorMessage  (HWND hwnd, LPCTSTR format, ...);
//(TOPMOST)
int TopErrorMessage  (HWND hwnd, LPCTSTR format, ...);
#define ErrorBeep()     MessageBeep(MB_ICONSTOP)

//警告：三角に「！」[OK]
int WarningMessage   (HWND hwnd, LPCTSTR format, ...);
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...);
#define WarningBeep()   MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」[OK]
int InfoMessage   (HWND hwnd, LPCTSTR format, ...);
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...);
#define InfoBeep()      MessageBeep(MB_ICONINFORMATION)

//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...);
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...);
#define ConfirmBeep()   MessageBeep(MB_ICONQUESTION)

//その他メッセージ表示用ボックス[OK]
int OkMessage  (HWND hwnd, LPCTSTR format, ...);
int TopOkMessage  (HWND hwnd, LPCTSTR format, ...);

//タイプ指定メッセージ表示用ボックス
int CustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);
//(TOPMOST)
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);

//作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...);

///////////////////////////////////////////////////////////////////////
#endif /* _DEBUG_H_ */


/*[EOF]*/
