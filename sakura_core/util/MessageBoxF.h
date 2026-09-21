/*!	@file
	@brief MessageBox用関数

	@author Norio Nakatani

	@date 2013/03/03 Uchi Debug1.hから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#ifndef SAKURA_MESSAGEBOXF_542C25FF_34EB_4920_AC1A_DA32919E101B_H_
#define SAKURA_MESSAGEBOXF_542C25FF_34EB_4920_AC1A_DA32919E101B_H_
#pragma once

#include "basis/primitive.h"
#include "util/string_ex.h"

#include <tchar.h>
#include <Windows.h>

#include <cstdarg>

#include <optional>
#include <string>

namespace basis {

std::wstring GetBugReportCaption();

} // namespace basis

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//2007.10.02 kobake メッセージボックスの使用はデバッグ時に限らないので、「Debug～」という名前を廃止
#undef MessageBox
#define MessageBox Wrap_MessageBox
int Wrap_MessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType);

/*!
 * @brief メッセージボックスを表示する
 *
 * @param[in] text メッセージボックスに表示する文字列
 * @param[in, opt] uType メッセージボックスのスタイル (省略時は OkOnly)
 * @param[in, opt] hWndOwner オーナーウィンドウのハンドル（省略時はメインウィンドウ）
 * @param[in, opt] optCaption メッセージボックスのタイトル（省略時は「アプリ名」）
 * @returns メッセージボックスがどのボタンで閉じられたかを示す値。IDYESなど。
 */
int MessageBoxS(
	const std::wstring& text,
	UINT uType = MB_OK,
	_In_opt_ HWND hWndOwner = nullptr,
	const std::optional<std::wstring>& optCaption = std::nullopt
);

//テキスト整形機能付きMessageBox

/*!
 * @brief 書式付きメッセージボックス
 *
 * 引数で与えられた情報をダイアログボックスで表示する．
 * デバッグ目的以外でも使用できる．
 *
 * @param[in] uType メッセージボックスのスタイル (MessageBoxと同じ形式)
 * @param[in, out] caption メッセージボックスのタイトル
 * @param[in, opt] hWndOwner オーナーウィンドウのハンドル
 * @param[in] format フォーマット文字列
 * @param[in, opt] args 引数リスト
 * @returns メッセージボックスがどのボタンで閉じられたかを示す値。IDYESなど。
 */
template <basis::NullTerminatedStringConstructible<WCHAR> A, typename... Args>
int MessageBoxF(
	HWND hWndOwner,									//!< [in, opt] オーナーウィンドウのハンドル
	UINT uType,										//!< [in] メッセージボックスのスタイル (MessageBoxと同じ形式)
	const A& caption,								//!< [in, opt] メッセージボックスのタイトル
	_In_z_ _Printf_format_string_ LPCWSTR format,	//!< [in] 表示するテキスト。printf仕様の書式指定が可能。
	const Args&... args								//!< [in,out] 引数リスト
)
{
	// メッセージタイトルはNUL終端文字列として扱う（ただし、省略可能。）
	const auto capStr = cxx::NullTerminatedString{ caption };

	// 出力文字列を整形する
	const auto text = strprintf(
		format,
		std::as_const(args)...
	);

	// メッセージボックスを表示する
	return MessageBoxS(
		text,	// 整形された文字列を参照渡しする
		uType,
		hWndOwner,
		capStr.optStr()	// 指定されている場合、ここで std::wstring が生成される
	);
}

//                ユーザー用メッセージボックス                   //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

//デバッグ用メッセージボックス
#define MYMESSAGEBOX MessageBoxF

//一般の警告音
#define DefaultBeep()   ::MessageBeep(MB_OK)

//エラー：赤丸に「×」[OK]
template <typename... Args> int ErrorMessage   (HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)		{ return MessageBoxS( strprintf(format, args...), MB_OK | MB_ICONSTOP,				hWndOwner); }
template <typename... Args> int TopErrorMessage(HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)		{ return MessageBoxS( strprintf(format, args...), MB_OK | MB_ICONSTOP | MB_TOPMOST, hWndOwner); }
#define ErrorBeep()     ::MessageBeep(MB_ICONSTOP)

//警告：三角に「！」[OK]
template <typename... Args> int WarningMessage   (HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)	{ return MessageBoxS( strprintf(format, args...), MB_OK | MB_ICONEXCLAMATION,			   hWndOwner); }
template <typename... Args> int TopWarningMessage(HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)	{ return MessageBoxS( strprintf(format, args...), MB_OK | MB_ICONEXCLAMATION | MB_TOPMOST, hWndOwner); }
#define WarningBeep()   ::MessageBeep(MB_ICONEXCLAMATION)

//情報：青丸に「i」[OK]
template <typename... Args> int InfoMessage   (HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)		{ return MessageBoxS( strprintf(format, args...), MB_OK | MB_ICONINFORMATION,			   hWndOwner); }
template <typename... Args> int TopInfoMessage(HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)		{ return MessageBoxS( strprintf(format, args...), MB_OK | MB_ICONINFORMATION | MB_TOPMOST, hWndOwner); }
#define InfoBeep()      ::MessageBeep(MB_ICONINFORMATION)

//確認：吹き出しの「？」 [はい][いいえ] 戻り値:IDYES,IDNO
template <typename... Args> int ConfirmMessage   (HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)	{ return MessageBoxS( strprintf(format, args...), MB_YESNO | MB_ICONQUESTION,			   hWndOwner); }
template <typename... Args> int TopConfirmMessage(HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)	{ return MessageBoxS( strprintf(format, args...), MB_YESNO | MB_ICONQUESTION | MB_TOPMOST, hWndOwner); }
#define ConfirmBeep()   ::MessageBeep(MB_ICONQUESTION)

//三択：吹き出しの「？」 [はい][いいえ][キャンセル]  戻り値:ID_YES,ID_NO,ID_CANCEL
template <typename... Args> int Select3Message   (HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)	{ return MessageBoxS( strprintf(format, args...), MB_YESNOCANCEL | MB_ICONQUESTION,				 hWndOwner); }
template <typename... Args> int TopSelect3Message(HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)	{ return MessageBoxS( strprintf(format, args...), MB_YESNOCANCEL | MB_ICONQUESTION | MB_TOPMOST, hWndOwner); }

//その他メッセージ表示用ボックス[OK]
template <typename... Args> int OkMessage   (HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)			{ return MessageBoxS( strprintf(format, args...), MB_OK,			  hWndOwner); }
template <typename... Args> int TopOkMessage(HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args)			{ return MessageBoxS( strprintf(format, args...), MB_OK | MB_TOPMOST, hWndOwner); }

//タイプ指定メッセージ表示用ボックス
template <typename... Args> int CustomMessage   (HWND hWndOwner, UINT uType, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args) { return MessageBoxS( strprintf(format, args...), uType,			    hWndOwner); }
template <typename... Args> int TopCustomMessage(HWND hWndOwner, UINT uType, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args) { return MessageBoxS( strprintf(format, args...), uType | MB_TOPMOST, hWndOwner); }

//作者に教えて欲しいエラー
template <typename... Args> int PleaseReportToAuthor(HWND hWndOwner, _In_z_ _Printf_format_string_ LPCWSTR format, const Args& ...args) { return MessageBoxS( strprintf(format, args...), MB_OK | MB_ICONSTOP | MB_TOPMOST, hWndOwner, basis::GetBugReportCaption()); }

#endif /* SAKURA_MESSAGEBOXF_542C25FF_34EB_4920_AC1A_DA32919E101B_H_ */
