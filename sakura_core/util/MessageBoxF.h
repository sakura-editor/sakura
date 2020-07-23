/*!	@file
	@brief MessageBox用関数

	@author Norio Nakatani

	@date 2013/03/03 Uchi Debug1.hから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

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

#pragma once

#include <stdarg.h>
#include <tchar.h>

#include <Windows.h>

#include "config/app_constants.h"

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * サクラエディタ独自MessageBox実装
 *
 * サクラエディタ独自の特殊な言語切替機構を実現するため、
 * メッセージボックスのUI言語を選択中の言語で上書きする。
 * コンソールモードではメッセージボックスを表示しない。
 *
 * @date 2007/10/02 kobake メッセージボックスの使用はデバッグ時に限らないので、「Debug～」という名前を廃止
 * @date 2020/07/23 berryzplus コンソールモードではメッセージボックスを表示しないように変更
 */
int Wrap_MessageBox(
	HWND		hWnd,		//!< [in,opt] メッセージボックスの親ウインドウ
	LPCWSTR		lpText,		//!< [in,opt] メッセージの本文
	LPCWSTR		lpCaption,	//!< [in,opt] メッセージの表題
	UINT		uType		//!< [in] MessageBox関数に渡すオプション
);

// Windows SDKの定義をundefして独自定義に差し替える
#undef MessageBox
#define MessageBox Wrap_MessageBox

//テキスト整形機能付きMessageBox
int VMessageBoxF( HWND hWnd, UINT uType, LPCWSTR lpCaption, LPCWSTR pszFormat, va_list argList );
int MessageBoxF( HWND hWnd, UINT uType, LPCWSTR lpCaption, LPCWSTR pszFormat, ... );

//                ユーザ用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF

//一般の警告音
#define DefaultBeep()   ::MessageBeep(MB_OK)

//エラー：赤丸に「×」[OK]
#define ErrorMessage( hWnd, format, ... )				MessageBoxF( (hWnd), MB_OK | MB_ICONSTOP,                      GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define TopErrorMessage( hWnd, format, ... )			MessageBoxF( (hWnd), MB_OK | MB_ICONSTOP | MB_TOPMOST,         GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define ErrorBeep()     ::MessageBeep(MB_ICONSTOP)

//警告：三角に「！」[OK]
#define WarningMessage( hWnd, format, ... )				MessageBoxF( (hWnd), MB_OK | MB_ICONEXCLAMATION,               GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define TopWarningMessage( hWnd, format, ... )			MessageBoxF( (hWnd), MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST,  GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define WarningBeep()   ::MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」[OK]
#define InfoMessage( hWnd, format, ... )				MessageBoxF( (hWnd), MB_OK | MB_ICONINFORMATION,               GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define TopInfoMessage( hWnd, format, ... )				MessageBoxF( (hWnd), MB_OK | MB_ICONINFORMATION | MB_TOPMOST,  GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define InfoBeep()      ::MessageBeep(MB_ICONINFORMATION)

//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
#define ConfirmMessage( hWnd, format, ... )				MessageBoxF( (hWnd), MB_YESNO | MB_ICONQUESTION,               GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define TopConfirmMessage( hWnd, format, ... )			MessageBoxF( (hWnd), MB_YESNO | MB_ICONQUESTION | MB_TOPMOST,  GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define ConfirmBeep()   ::MessageBeep(MB_ICONQUESTION)

//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
#define Select3Message( hWnd, format, ... )				MessageBoxF( (hWnd), MB_YESNOCANCEL | MB_ICONQUESTION,              GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define TopSelect3Message( hWnd, format, ... )			MessageBoxF( (hWnd), MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST, GSTR_APPNAME, (format), ##__VA_ARGS__ )

//その他メッセージ表示用ボックス[OK]
#define OkMessage( hWnd, format, ... )					MessageBoxF( (hWnd), MB_OK,              GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define TopOkMessage( hWnd, format, ... )				MessageBoxF( (hWnd), MB_OK | MB_TOPMOST, GSTR_APPNAME, (format), ##__VA_ARGS__ )

//タイプ指定メッセージ表示用ボックス
#define CustomMessage( hWnd, uType, format, ... )		MessageBoxF( (hWnd), (uType),              GSTR_APPNAME, (format), ##__VA_ARGS__ )
#define TopCustomMessage( hWnd, uType, format, ... )	MessageBoxF( (hWnd), (uType) | MB_TOPMOST, GSTR_APPNAME, (format), ##__VA_ARGS__ )

//作者に教えて欲しいエラー
#define PleaseReportToAuthor( hWnd, format, ... )		MessageBoxF( (hWnd), MB_OK | MB_ICONSTOP | MB_TOPMOST, LS(STR_ERR_DLGDOCLMN1), (format), ##__VA_ARGS__ )
