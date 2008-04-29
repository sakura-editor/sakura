/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

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
SAKURA_CORE_API void DebugOutW( LPCWSTR lpFmt, ...);
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
SAKURA_CORE_API int VMessageBoxF_W( HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, va_list& v );
SAKURA_CORE_API int VMessageBoxF_A( HWND hwndOwner, UINT uType, LPCSTR  lpCaption, LPCSTR  lpText, va_list& v );
SAKURA_CORE_API int MessageBoxF_W ( HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, ... );
SAKURA_CORE_API int MessageBoxF_A ( HWND hwndOwner, UINT uType, LPCSTR  lpCaption, LPCSTR  lpText, ... );

//TCHAR
#ifdef _UNICODE
	#define VMessageBoxF VMessageBoxF_W
	#define MessageBoxF  MessageBoxF_W
#else
	#define VMessageBoxF VMessageBoxF_A
	#define MessageBoxF  MessageBoxF_A
#endif

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
	#define MYTRACE_W DebugOutW
#endif
#ifndef _DEBUG
	#define MYTRACE   Do_not_use_the_MYTRACE_function_if_release_mode
	#define MYTRACE_A Do_not_use_the_MYTRACE_A_function_if_release_mode
	#define MYTRACE_W Do_not_use_the_MYTRACE_A_function_if_release_mode
#endif

//#ifdef _DEBUG～#endifで囲まなくても良い版
#ifdef _DEBUG
#define DBPRINT_A DebugOutA
#define DBPRINT_W DebugOutW
#else
#define DBPRINT_A(...)
#define DBPRINT_W(...)
#endif

#ifdef _UNICODE
#define DBPRINT DBPRINT_W
#else
#define DBPRINT DBPRINT_A
#endif

//RELEASE版でも出力する版 (RELEASEでのみ発生するバグを監視する目的)
#define RELPRINT_A DebugOutA

#define MYASSERT AssertError


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//               デバッグ用メッセージボックス                  //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//超簡易版  2007.09.29 kobake 作成
void DBMSG_IMP(const ACHAR* msg); //!< メッセージボックスを表示。キャプションにはexe名。

#ifdef _DEBUG
#define DBMSG DBMSG_IMP
#else
#define DBMSG(S)
#endif

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                ユーザ用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//$$メモ：Debug.h以外の置き場所を考える

//デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF
#define MYMESSAGEBOX_A MessageBoxF_A

//エラー：赤丸に「×」
#define ErrorMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK | MB_ICONSTOP						, GSTR_APPNAME,   format, __VA_ARGS__)
#define ErrorMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_ICONSTOP						, GSTR_APPNAME_A, format, __VA_ARGS__)
//(TOPMOST)
#define TopErrorMessage(hwnd, format, ...)		MessageBoxF		(hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST			, GSTR_APPNAME,   format, __VA_ARGS__)
#define TopErrorMessage_A(hwnd, format, ...)	MessageBoxF_A	(hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST			, GSTR_APPNAME_A, format, __VA_ARGS__)
#define ErrorBeep()								MessageBeep(MB_ICONSTOP)

//警告：三角に「！」
#define WarningMessage(hwnd, format, ...)		MessageBoxF		(hwnd, MB_OK | MB_ICONEXCLAMATION				, GSTR_APPNAME,   format, __VA_ARGS__)
#define WarningMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_ICONEXCLAMATION				, GSTR_APPNAME_A, format, __VA_ARGS__)
#define TopWarningMessage(hwnd, format, ...)	MessageBoxF		(hwnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST	, GSTR_APPNAME,   format, __VA_ARGS__)
#define WarningBeep()							MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」
#define InfoMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK | MB_ICONINFORMATION				, GSTR_APPNAME,   format, __VA_ARGS__)
#define InfoMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_ICONINFORMATION				, GSTR_APPNAME_A, format, __VA_ARGS__)
#define TopInfoMessage(hwnd, format, ...)		MessageBoxF		(hwnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST	, GSTR_APPNAME,   format, __VA_ARGS__)
#define InfoBeep()								MessageBeep(MB_ICONINFORMATION)

//その他メッセージ表示用ボックス
#define OkMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK									, GSTR_APPNAME,   format, __VA_ARGS__)
#define OkMessage_A(hwnd, format, ...)			MessageBoxF_A	(hwnd, MB_OK									, GSTR_APPNAME_A, format, __VA_ARGS__)
//(TOPMOST)
#define TopOkMessage(hwnd, format, ...)			MessageBoxF		(hwnd, MB_OK | MB_TOPMOST						, GSTR_APPNAME,   format, __VA_ARGS__)
#define TopOkMessage_A(hwnd, format, ...)		MessageBoxF_A	(hwnd, MB_OK | MB_TOPMOST						, GSTR_APPNAME_A, format, __VA_ARGS__)

//作者に教えて欲しいエラー
#define PleaseReportToAuthor(hwnd, format, ...)	MessageBoxF		(hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST	, _T("作者に教えて欲しいエラー"), format, __VA_ARGS__)


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                         フラグ等                            //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
#ifdef _DEBUG
	//!	設定している場所はあるが，参照している場所がない変数
	SAKURA_CORE_API extern int gm_ProfileOutput;
#endif



#include "debug/Debug2.h"
#include "debug/Debug3.h"

///////////////////////////////////////////////////////////////////////
#endif /* _DEBUG_H_ */



