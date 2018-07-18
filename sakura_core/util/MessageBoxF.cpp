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
#include <stdarg.h>
#include <tchar.h>
#include "MessageBoxF.h"
#include "window/CEditWnd.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
int Wrap_MessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
	static int (WINAPI *RealMessageBox)(HWND, LPCTSTR, LPCTSTR, UINT, WORD);
	static HMODULE hMod = NULL;
	if( hMod == NULL ){	// 初期化されていない場合は初期化する
	hMod = GetModuleHandle(_T("USER32"));
#ifdef _UNICODE
		*(FARPROC *)&RealMessageBox = GetProcAddress(hMod, "MessageBoxExW");
#else
		*(FARPROC *)&RealMessageBox = GetProcAddress(hMod, "MessageBoxExA");
#endif
	}

	// lpText, lpCaption をローカルバッファにコピーして MessageBox API を呼び出す
	// ※ 使い回しのバッファが使用されていてそれが裏で書き換えられた場合でも
	//    メッセージボックス上の Ctrl+C が文字化けしないように
	return RealMessageBox(hWnd, lpText? std::tstring(lpText).c_str(): NULL,
		lpCaption? std::tstring(lpCaption).c_str(): NULL, uType, CSelectLang::getDefaultLangId());
}

HWND GetMessageBoxOwner(HWND hwndOwner)
{
	if(hwndOwner==NULL && g_pcEditWnd){
		return g_pcEditWnd->GetHwnd();
	}
	else{
		return hwndOwner;
	}
}

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
	va_list&	v			//!< [in,out] 引数リスト
)
{
	hwndOwner=GetMessageBoxOwner(hwndOwner);
	//整形
	static TCHAR szBuf[16000];
	tchar_vsnprintf_s(szBuf,_countof(szBuf),lpText,v);
	//API呼び出し
	return ::MessageBox( hwndOwner, szBuf, lpCaption, uType);
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
int ErrorMessage   (HWND hwnd, LPCTSTR format, ...){      va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP                     , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopErrorMessage(HWND hwnd, LPCTSTR format, ...){      va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST        , GSTR_APPNAME,   format, p);va_end(p);return n;}	//(TOPMOST)

//警告：三角に「i」
int WarningMessage   (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopWarningMessage(HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//情報：青丸に「i」
int InfoMessage   (HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopInfoMessage(HWND hwnd, LPCTSTR format, ...){       va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONINFORMATION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//確認：吹き出しの「？」 戻り値:ID_YES,ID_NO
int ConfirmMessage   (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION              , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopConfirmMessage(HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNO | MB_ICONQUESTION | MB_TOPMOST , GSTR_APPNAME,   format, p);va_end(p);return n;}

//三択：吹き出しの「？」 戻り値:ID_YES,ID_NO,ID_CANCEL
int Select3Message   (HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNOCANCEL | MB_ICONQUESTION              , GSTR_APPNAME, format, p);va_end(p);return n;}
int TopSelect3Message(HWND hwnd, LPCTSTR format, ...){    va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST , GSTR_APPNAME, format, p);va_end(p);return n;}

//その他メッセージ表示用ボックス
int OkMessage   (HWND hwnd, LPCTSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK                                   , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopOkMessage(HWND hwnd, LPCTSTR format, ...){         va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_TOPMOST                      , GSTR_APPNAME,   format, p);va_end(p);return n;}	//(TOPMOST)

//タイプ指定メッセージ表示用ボックス
int CustomMessage   (HWND hwnd, UINT uType, LPCTSTR format, ...){   va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType                         , GSTR_APPNAME,   format, p);va_end(p);return n;}
int TopCustomMessage(HWND hwnd, UINT uType, LPCTSTR format, ...){   va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, uType | MB_TOPMOST            , GSTR_APPNAME,   format, p);va_end(p);return n;}	//(TOPMOST)

//作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCTSTR format, ...){ va_list p;va_start(p, format);int n=VMessageBoxF  (hwnd, MB_OK | MB_ICONSTOP | MB_TOPMOST, LS(STR_ERR_DLGDOCLMN1), format, p);va_end(p);return n;}



