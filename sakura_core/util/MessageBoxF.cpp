/*!	@file
	@brief MessageBox用関数

	@author Norio Nakatani

	@date 2002/01/17 aroka 型の修正
	@date 2013/03/03 Uchi Debug1.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka

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

#include "StdAfx.h"
#include "MessageBoxF.h"
#include "_main/CProcess.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * サクラエディタ独自MessageBox実装
 */
int Wrap_MessageBox(
	HWND		hWnd,		//!< [in,opt] メッセージボックスの親ウインドウ
	LPCWSTR		lpText,		//!< [in,opt] メッセージの本文
	LPCWSTR		lpCaption,	//!< [in,opt] メッセージの表題
	UINT		uType		//!< [in] MessageBox関数に渡すオプション
)
{
	// メッセージボックスの表示結果(通常は閉じるときに押下されたボタンのID)
	int nResult = IDOK;

	// lpTextの文字列長を求める
	DWORD dwTextLength = lpText ? ::wcslen( lpText ) : 0;

	// 標準エラー出力を取得する
	HANDLE hStdErr = ::GetStdHandle( STD_ERROR_HANDLE );
	if( hStdErr ){
		// lpText を標準エラー出力に書き出す
		DWORD dwWritten = 0;
		::WriteConsoleW( hStdErr, lpText, dwTextLength, &dwWritten, NULL );

		// メッセージボックスの種類に応じて、一番右端のボタンIDを返す。
		switch (uType & MB_TYPEMASK){
		case MB_OK:					nResult = IDOK;       break;
		case MB_OKCANCEL:			nResult = IDCANCEL;   break;
		case MB_ABORTRETRYIGNORE:	nResult = IDIGNORE;   break;
		case MB_YESNOCANCEL:		nResult = IDCANCEL;   break;
		case MB_YESNO:				nResult = IDNO;       break;
		case MB_RETRYCANCEL:		nResult = IDCANCEL;   break;
		case MB_CANCELTRYCONTINUE:	nResult = IDCONTINUE; break;
		}
	}else{
		// メッセージボックスの親ウインドウハンドルを補正する
		if( hWnd == NULL ){
			const auto* pcProcess = CProcess::getInstance();
			if( pcProcess ){
				hWnd = pcProcess->GetMainWindow();
			}
		}

		// メッセージボックスの表題を補正する
		if ( lpCaption == NULL ){
			lpCaption = GSTR_APPNAME;
		}

		// 選択中の言語IDを取得する
		LANGID wLangId = CSelectLang::getDefaultLangId();

		// Windows API MessageBoxEx を呼び出してダイアログを表示する
		nResult = ::MessageBoxEx( hWnd, lpText, lpCaption, uType, wLangId );
	}

	return nResult;
}

/*!
	書式付きメッセージボックス(arg_list版)

	引数で与えられた情報をダイアログボックスで表示する．
	デバッグ目的以外でも使用できる．
*/
int VMessageBoxF(
	HWND		hWnd,		//!< [in,opt] メッセージボックスの親ウインドウ
	UINT		uType,		//!< [in] MessageBox関数に渡すオプション
	LPCWSTR		lpCaption,	//!< [in,opt] メッセージの表題
	LPCWSTR		pszFormat,	//!< [in] 表示するテキスト。printf仕様の書式指定が可能。
	va_list		argList		//!< [in] 引数リスト
)
{
	// 整形用バッファは static に確保する。
	// MessageBox関数はスレッドをブロックするので、再入は考慮しない。
	static WCHAR szBuf[16000];

	// メッセージを整形する
	::_vsnwprintf_s( szBuf, _TRUNCATE, pszFormat, argList );

	// Windows API を呼び出してダイアログを表示する
	return Wrap_MessageBox( hWnd, szBuf, lpCaption, uType );
}

/*!
	書式付きメッセージボックス(可変長引数版)

	引数で与えられた情報をダイアログボックスで表示する．
	デバッグ目的以外でも使用できる．
*/
int MessageBoxF(
	HWND		hWnd,		//!< [in,opt] メッセージボックスの親ウインドウ
	UINT		uType,		//!< [in] MessageBox関数に渡すオプション
	LPCWSTR		lpCaption,	//!< [in,opt] メッセージの表題
	LPCWSTR		pszFormat,	//!< [in] 表示するテキスト。printf仕様の書式指定が可能。
	/*__VA_ARGS__*/ ...
)
{
	va_list argptr;
	va_start( argptr, pszFormat );
	int nRet = VMessageBoxF( hWnd, uType, lpCaption, pszFormat, argptr );
	va_end( argptr );
	return nRet;
}

//エラー：赤丸に「×」[OK]
int ErrorMessage   (HWND hwnd, LPCWSTR format, ...){      va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP                     , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopErrorMessage(HWND hwnd, LPCWSTR format, ...){      va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST        , GSTR_APPNAME,   format, p);va_end(p);return n;}	//(TOPMOST)

//警告：三角に「i」
int WarningMessage   (HWND hwnd, LPCWSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopWarningMessage(HWND hwnd, LPCWSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//情報：青丸に「i」
int InfoMessage   (HWND hwnd, LPCWSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopInfoMessage(HWND hwnd, LPCWSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//確認：吹き出しの「？」 戻り値:ID_YES,ID_NO
int ConfirmMessage   (HWND hwnd, LPCWSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopConfirmMessage(HWND hwnd, LPCWSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//三択：吹き出しの「？」 戻り値:ID_YES,ID_NO,ID_CANCEL
int Select3Message   (HWND hwnd, LPCWSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNOCANCEL | MB_ICONQUESTION              , GSTR_APPNAME, format, p);va_end(p);return n;}
int TopSelect3Message(HWND hwnd, LPCWSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST , GSTR_APPNAME, format, p);va_end(p);return n;}

//その他メッセージ表示用ボックス
int OkMessage   (HWND hwnd, LPCWSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK                                   , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopOkMessage(HWND hwnd, LPCWSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_TOPMOST                      , GSTR_APPNAME,   format, p);va_end(p);return n;}	//(TOPMOST)

//タイプ指定メッセージ表示用ボックス
int CustomMessage   (HWND hwnd, UINT uType, LPCWSTR format, ...){   va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType                         , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopCustomMessage(HWND hwnd, UINT uType, LPCWSTR format, ...){   va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType | MB_TOPMOST            , GSTR_APPNAME,   format, p);va_end(p);return n;}	//(TOPMOST)

//作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCWSTR format, ...){ va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST, LS(STR_ERR_DLGDOCLMN1), format, p);va_end(p);return n;}
