/*!	@file
	@brief MessageBox用関数

	@author Norio Nakatani

	@date 2002/01/17 aroka 型の修正
	@date 2013/03/03 Uchi Debug1.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "MessageBoxF.h"

#include <iostream>

#include "_main/CProcess.h"
#include "window/CEditWnd.h"
#include "CSelectLang.h"
#include "config/app_constants.h"
#include "String_define.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
int Wrap_MessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	// 選択中の言語IDを取得する
	LANGID wLangId = CSelectLang::getDefaultLangId();

	// 標準エラー出力が存在する場合
	if(::GetStdHandle(STD_ERROR_HANDLE)){
		// lpText を標準エラー出力に書き出す
		std::clog << (lpText ? wcstou8s(lpText) : "") << std::endl;

		// いい加減な戻り値を返す。(返り値0は未定義なので本来返らない値を返している)
		return 0;
	}

	// lpText, lpCaption をローカルバッファにコピーして MessageBox API を呼び出す
	// ※ 使い回しのバッファが使用されていてそれが裏で書き換えられた場合でも
	//    メッセージボックス上の Ctrl+C が文字化けしないように
	return ::MessageBoxEx(hWnd,
		lpText ? std::wstring(lpText).c_str() : NULL,
		lpCaption ? std::wstring(lpCaption).c_str() : NULL,
		uType,
		wLangId
	);
}

/*!
 * メッセージボックスのオーナーウインドウを取得します。
 *
 * @param [in,opt] hWndOwner オーナーウインドウ
 * @returns メッセージボックスのオーナーウインドウ
 * @retval 指定したオーナーウインドウが不正値で、プロセスのメインウインドウが作成済みの場合
 * @retval hWndOwner 指定したオーナーウインドウが適正値で、メインウインドウが未作成の場合
 */
HWND GetMessageBoxOwner(HWND hWndOwner)
{
	if( !hWndOwner || !::IsWindow(hWndOwner) )
	{
		if( const auto pcProcess = CProcess::getInstance() )
		{
			hWndOwner = pcProcess->GetMainWindow();
		}
	}
	return hWndOwner;
}

/*!
	書式付きメッセージボックス

	引数で与えられた情報をダイアログボックスで表示する．
	デバッグ目的以外でも使用できる．
*/
int VMessageBoxF(
	HWND		hwndOwner,	//!< [in] オーナーウィンドウのハンドル
	UINT		uType,		//!< [in] メッセージボックスのスタイル (MessageBoxと同じ形式)
	LPCWSTR		lpCaption,	//!< [in] メッセージボックスのタイトル
	LPCWSTR		lpText,		//!< [in] 表示するテキスト。printf仕様の書式指定が可能。
	va_list&	v			//!< [in,out] 引数リスト
)
{
	const auto buf = vstrprintf(lpText,v);
	if (!hwndOwner) {
		hwndOwner = GetMessageBoxOwner(hwndOwner);
	}
	return ::MessageBox(hwndOwner, buf.data(), lpCaption, uType);
}

int MessageBoxF( HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, ... )
{
	va_list v;
	va_start(v,lpText);
	int nRet = VMessageBoxF(hwndOwner, uType, lpCaption, lpText, v);
	va_end(v);
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
