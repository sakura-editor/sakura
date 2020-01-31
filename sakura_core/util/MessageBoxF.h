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

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake メッセージボックスの使用はデバッグ時に限らないので、「Debug～」という名前を廃止
#undef MessageBox
#define MessageBox Wrap_MessageBox
int Wrap_MessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

//テキスト整形機能付きMessageBox
int VMessageBoxF(HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, va_list &v);
int MessageBoxF(HWND hwndOwner, UINT uType, LPCWSTR lpCaption, LPCWSTR lpText, ...);

//                ユーザ用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF

//一般の警告音
#define DefaultBeep() ::MessageBeep(MB_OK)

//エラー：赤丸に「×」[OK]
int ErrorMessage(HWND hwnd, LPCWSTR format, ...);
int TopErrorMessage(HWND hwnd, LPCWSTR format, ...); //(TOPMOST)
#define ErrorBeep() ::MessageBeep(MB_ICONSTOP)

//警告：三角に「！」[OK]
int WarningMessage(HWND hwnd, LPCWSTR format, ...);
int TopWarningMessage(HWND hwnd, LPCWSTR format, ...);
#define WarningBeep() ::MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」[OK]
int InfoMessage(HWND hwnd, LPCWSTR format, ...);
int TopInfoMessage(HWND hwnd, LPCWSTR format, ...);
#define InfoBeep() ::MessageBeep(MB_ICONINFORMATION)

//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
int ConfirmMessage(HWND hwnd, LPCWSTR format, ...);
int TopConfirmMessage(HWND hwnd, LPCWSTR format, ...);
#define ConfirmBeep() ::MessageBeep(MB_ICONQUESTION)

//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
int Select3Message(HWND hwnd, LPCWSTR format, ...);
int TopSelect3Message(HWND hwnd, LPCWSTR format, ...);

//その他メッセージ表示用ボックス[OK]
int OkMessage(HWND hwnd, LPCWSTR format, ...);
int TopOkMessage(HWND hwnd, LPCWSTR format, ...);

//タイプ指定メッセージ表示用ボックス
int CustomMessage(HWND hwnd, UINT uType, LPCWSTR format, ...);
int TopCustomMessage(HWND hwnd, UINT uType, LPCWSTR format, ...); //(TOPMOST)

//作者に教えて欲しいエラー
int PleaseReportToAuthor(HWND hwnd, LPCWSTR format, ...);
