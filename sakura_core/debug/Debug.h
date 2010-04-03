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
#if (defined(_MSC_VER) && 1400 <= _MSC_VER) || (defined(__GNUC__) && 3 <= __GNUC__ )
#define DBPRINT_A(...)
#define DBPRINT_W(...)
#else
// Not support C99 variable macro
inline void DBPRINT_A( ... ){};
inline void DBPRINT_W( ... ){};
#endif
 
#endif

#ifdef _UNICODE
#define DBPRINT DBPRINT_W
#else
#define DBPRINT DBPRINT_A
#endif

#define DEBUG_TRACE DBPRINT


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


//エラー：赤丸に「×」[OK]
int ErrorMessage  (HWND hwnd, LPCTSTR format, ...);
int ErrorMessage_A(HWND hwnd, LPCSTR  format, ...);
//(TOPMOST)
int TopErrorMessage  (HWND hwnd, LPCTSTR format, ...);
int TopErrorMessage_A(HWND hwnd, LPCSTR format, ...);
#define ErrorBeep()     MessageBeep(MB_ICONSTOP)

//警告：三角に「！」[OK]
int WarningMessage   (HWND hwnd, LPCTSTR format, ...);
int WarningMessage_A (HWND hwnd, LPCSTR  format, ...);
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...);
#define WarningBeep()   MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」[OK]
int InfoMessage   (HWND hwnd, LPCTSTR format, ...);
int InfoMessage_A (HWND hwnd, LPCSTR  format, ...);
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...);
#define InfoBeep()      MessageBeep(MB_ICONINFORMATION)

//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...);
int ConfirmMessage_A (HWND hwnd, LPCSTR  format, ...);
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...);
#define ConfirmBeep()   MessageBeep(MB_ICONQUESTION)

//その他メッセージ表示用ボックス[OK]
int OkMessage  (HWND hwnd, LPCTSTR format, ...);
int OkMessage_A(HWND hwnd, LPCSTR  format, ...);
int TopOkMessage  (HWND hwnd, LPCTSTR format, ...);
int TopOkMessage_A(HWND hwnd, LPCSTR format, ...);

//タイプ指定メッセージ表示用ボックス
int CustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);
int CustomMessage_A(HWND hwnd, UINT uType, LPCSTR format, ...);
//(TOPMOST)
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...);
int TopCustomMessage_A(HWND hwnd, UINT uType, LPCSTR format, ...);

//作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...);



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



