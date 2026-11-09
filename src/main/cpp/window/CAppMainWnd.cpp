/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "window/CAppMainWnd.hpp"

#include "util/os/make_system_error.hpp"

#include "CSelectLang.h"

/*!
 * @brief コンストラクタ
 *
 * @param[in] hInstance インスタンスハンドル
 */
CAppMainWnd::CAppMainWnd(
	_In_ HINSTANCE hInstance
)
	: m_hInstance(hInstance)
{
	m_hIcons.Create(m_hInstance);

	m_cMenuDrawer.Create(
		CSelectLang::getLangRsrcInstance(),
		GetHwnd(),	// 常にNULLなので、指定する意味はない。
		&GetIcons()
	);
}

/*!
 * @brief メインウィンドウのメッセージループ
 *
 * @returns 終了コード
 */
EAppExitCode CAppMainWnd::MessageLoop() const
{
	// メッセージを受け取るためのMSG構造体
	MSG	msg{};
	
	// メッセージループ
	for (;;)
	{
		// 次のメッセージが来るまで無限に待機する
		const auto result = ::GetMessageW(&msg, nullptr, 0, 0);

		//WM_QUITが来たらループを抜ける
		if (0 == result) break;

		//GetMessageW処理中のエラーを検出したら例外を投げる
		if (result < 0) throw cxx::make_system_error("GetMessageW() failed");

		::TranslateMessage(&msg);
		::DispatchMessageW(&msg);
	}

	// PostQuitMessageに指定された終了コードを返す
	return static_cast<EAppExitCode>(msg.wParam);
}

/*!
 * @brief メインウィンドウのメッセージ配送
 *
 * @param hWnd [in] 宛先ウインドウのハンドル
 * @param uMsg [in] メッセージコード
 * @param wParam [in, opt] 第1パラメーター
 * @param lParam [in, opt] 第2パラメーター
 * @returns 処理結果 メッセージコードにより意味が異なる。（おおむね、0を返したら「処理済み」を示す。）
 */
LRESULT CAppMainWnd::DispatchEvent(
	HWND hWnd,
	UINT uMsg,
	WPARAM wParam,
	LPARAM lParam
)
{
	//あとはデフォルトに任せる
	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
