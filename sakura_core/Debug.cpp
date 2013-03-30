/*!	@file
	@brief デバッグ用関数

	@author Norio Nakatani

	@date 2001/06/23 N.Nakatani DebugOut()に微妙～な修正
	@date 2002/01/17 aroka 型の修正
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "Debug.h"
#include <tchar.h>

#ifdef _DEBUG
	int gm_ProfileOutput = 0;
#endif


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                   メッセージ出力：実装                      //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //


/*! @brief 書式付きデバッガ出力

	@param[in] lpFmt printfの書式付き文字列

	引数で与えられた情報をDebugStringとして出力する．
*/
void DebugOutA( LPCSTR lpFmt, ...)
{
	//整形
	static CHAR szText[16000];
	va_list argList;
	va_start(argList, lpFmt);
	::wvsprintf( szText, lpFmt, argList );
	::OutputDebugString( szText );

	//ウェイト
	::Sleep(1);	// Norio Nakatani, 2001/06/23 大量にトレースするときのために

	va_end(argList);
	return;
}


// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
/*!
	書式付きメッセージボックス

	引数で与えられた情報をダイアログボックスで表示する．
	デバッグ目的以外でも使用できる．
*/
int VMessageBoxF(
	HWND		hwndOwner,	//!< [in] オーナーウィンドウのハンドル
	UINT		uType,		//!< [in] メッセージボックスのスタイル (MessageBoxと同じ形式)
	LPCTSTR		lpCaption,	//!< [in] メッセージボックスのタイトル
	LPCTSTR		lpText,		//!< [in] 表示するテキスト。printf仕様の書式指定が可能。
	va_list&	v			//!< [in/out] 引数リスト
)
{
	static TCHAR szBuf[16000];
	::wvsprintf( szBuf, lpText, v );
	return ::MessageBox( hwndOwner, szBuf, lpCaption, uType );
}

int MessageBoxF( HWND hwndOwner, UINT uType, LPCTSTR lpCaption, LPCTSTR lpText, ... )
{
	va_list v;
	va_start(v,lpText);
	int nRet = VMessageBoxF(hwndOwner, uType, lpCaption, lpText, v);
	va_end(v);
	return nRet;
}


//エラー：赤丸に「×」[OK]
int ErrorMessage  (HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP                     , GSTR_APPNAME,   format, p);va_end(p);return n;}
//(TOPMOST)
int TopErrorMessage  (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST        , GSTR_APPNAME,   format, p);va_end(p);return n;}

//警告：三角に「i」
int WarningMessage   (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//情報：青丸に「i」
int InfoMessage   (HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//確認：吹き出しの「？」 戻り値:ID_YES,ID_NO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//その他メッセージ表示用ボックス
int OkMessage(HWND hwnd, LPCTSTR format, ...){            va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK                                   , GSTR_APPNAME,   format, p);va_end(p);return n;}
//(TOPMOST)
int TopOkMessage(HWND hwnd, LPCTSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_TOPMOST                      , GSTR_APPNAME,   format, p);va_end(p);return n;}

//タイプ指定メッセージ表示用ボックス
int CustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...){            va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType                   , GSTR_APPNAME,   format, p);va_end(p);return n;}
//(TOPMOST)
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType | MB_TOPMOST      , GSTR_APPNAME,   format, p);va_end(p);return n;}

//作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...){ va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST, _T("作者に教えて欲しいエラー"), format, p);va_end(p);return n;}

/*[EOF]*/
