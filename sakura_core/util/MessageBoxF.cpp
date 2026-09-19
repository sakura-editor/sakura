/*!	@file
	@brief MessageBox用関数

	@author Norio Nakatani

	@date 2002/01/17 aroka 型の修正
	@date 2013/03/03 Uchi Debug1.cppから分離
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, aroka
	Copyright (C) 2018-2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "MessageBoxF.h"

#include "_main/CProcess.h"

#include "config/app_constants.h"
#include "util/os.h"
#include "util/tchar_convert.h"
#include "window/CEditWnd.h"

#include "CSelectLang.h"

#include <iostream>

HWND GetMessageBoxOwner(HWND hWndOwner);

namespace basis {

/*!
 * @brief 作者に教えて欲しいエラーのタイトル
 *
 * 既存コード互換用。
 *
 * サクラエディタはもともと個人開発のアプリで、
 * 有志数人の共同開発時代を経て sourceforge プロジェクトとなった。
 *
 * いまとなっては必要性に疑問のある仕様だが、一旦残しておく。
 */
std::wstring GetBugReportCaption()
{
	// L"sakura: 作者に教えて欲しいエラー"
	return LS(STR_ERR_DLGDOCLMN1);
}

} // namespace basis

// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //
//                 メッセージボックス：実装                    //
// -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- //

/*!
 * メッセージボックスを表示します。
 *
 * @note 直接呼ばないでください。
 */
int User32::MessageBoxExW(
	_In_opt_ HWND hWnd,
	_In_opt_ LPCWSTR lpText,
	_In_opt_ LPCWSTR lpCaption,
	_In_ UINT uType,
	_In_ WORD wLanguageId
) const
{
	// 標準エラー出力が存在する場合
	if(::GetStdHandle(STD_ERROR_HANDLE)){
		// lpText を標準エラー出力に書き出す
		std::clog << (lpText ? cxx::to_string(lpText, CP_UTF8) : "") << std::endl;

		// いい加減な戻り値を返す。(返り値0は未定義なので本来返らない値を返している)
		return 0;
	}

	// 本物のAPIを呼ぶ。構造的にテスト不能なので、いつか代替策を考える必要がある。
	return ::MessageBoxExW(hWnd, lpText, lpCaption, uType, wLanguageId);
}

/*!
 * @brief メッセージボックスを表示する
 *
 * サクラエディタで使いやすいようにWindows APIをカスタムしたもの。
 *
 * @param[in] text メッセージボックスに表示する文字列
 * @param[in, opt] uType メッセージボックスのスタイル (省略時は OkOnly)
 * @param[in, opt] hWndOwner オーナーウィンドウのハンドル（省略時はメインウィンドウ）
 * @param[in, opt] optCaption メッセージボックスのタイトル（省略時は「アプリ名」）
 * @returns メッセージボックスがどのボタンで閉じられたかを示す値。IDYESなど。
 */
int MessageBoxS(
	const std::wstring& text,						// NUL終端を保証したいため、あえて std::wstring にする
	UINT uType,
	_In_opt_ HWND hWndOwner,
	const std::optional<std::wstring>& optCaption	// NUL終端を保証したいため、あえて std::wstring にする
)
{
	// メッセージボックスはモーダルダイアログなので、オーナーを指定する
	hWndOwner = GetMessageBoxOwner(hWndOwner);

	// キャプションを省略したら「アプリ名」を使う
	const auto capStr = optCaption.value_or(GSTR_APPNAME);

	// 現在の言語IDを取得する
	const auto langId = CSelectLang::getDefaultLangId();

	// Windows API を呼び出す
	return User32::getInstance()->MessageBoxExW(
		hWndOwner,
		text.c_str(),	// 非NULLのNUL終端文字列を渡す。
		capStr.c_str(),	// 非NULLのNUL終端文字列を渡す。
		uType,
		langId
	);
}

/*!
 * メッセージボックスを表示します。
 *
 * @note 直接呼ばないでください。
 * @note ヘッダー側でWindows SDKのマクロMessageBoxをWrap_MessageBoxに定義し直している。
 */
int Wrap_MessageBox(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	// メッセージ本文はNUL終端文字列として扱う
	const auto text = cxx::NullTerminatedString{ lpText };

	// メッセージタイトルはNUL終端文字列として扱う（ただし、省略可能。）
	const auto capStr = cxx::NullTerminatedString{ lpCaption };

	// lpText, lpCaption をローカルバッファにコピーして MessageBox API を呼び出す
	// ※ 使い回しのバッファが使用されていてそれが裏で書き換えられた場合でも
	//    メッセージボックス上の Ctrl+C が文字化けしないように

	// メッセージボックスを表示する
	return MessageBoxS(
		std::wstring{ text.str() },	// NUL終端を保証するため、ここでコピーする
		uType,
		hWnd,
		capStr.optStr()	// 指定されている場合、ここで std::wstring が生成される
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
