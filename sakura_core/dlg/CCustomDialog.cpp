/*! @file */
/*
	Copyright (C) 2023, Sakura Editor Organization

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
#include "dlg/CCustomDialog.hpp"

/*!
 * DialogProc(ダイアログのメッセージ配送)
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
INT_PTR CALLBACK CCustomDialog::DialogProc(
	HWND hDlg,		// handle to dialog box
	UINT uMsg,		// message
	WPARAM wParam,	// first message parameter
	LPARAM lParam 	// second message parameter
)
{
	// GetWindowLongPtrの引数にNULLはマズい
	if (!hDlg || !::IsWindow(hDlg))
	{
		return FALSE;
	}

	// WM_INITDIALOGが来たらウインドウハンドルにクラスインスタンスを紐付ける
	if (uMsg == WM_INITDIALOG && lParam)
	{
		auto pcDlg = std::bit_cast<CCustomDialog*>(lParam);

		::SetWindowLongPtrW(hDlg, DWLP_USER, lParam);

		pcDlg->m_hWnd = hDlg;

		return pcDlg->DispatchDlgEvent(hDlg, uMsg, wParam, lParam);
	}

	// GetWindowLongPtrでインスタンスを取り出し、処理させる
	if (auto pcDlg = std::bit_cast<CCustomDialog*>(::GetWindowLongPtrW(hDlg, DWLP_USER));
		pcDlg && pcDlg->m_hWnd == hDlg)
	{
		const auto ret = pcDlg->DispatchDlgEvent(hDlg, uMsg, wParam, lParam);

		// WM_NCDESTROYが来たらウインドウハンドルとインスタンスの紐付けを解除する
		if (uMsg == WM_NCDESTROY)
		{
			pcDlg->m_hWnd = nullptr;

			::SetWindowLongPtrW(hDlg, DWLP_USER, 0);
		}

		return ret;
	}

	return FALSE;
}

/*!
 * コンストラクタ
 */
CCustomDialog::CCustomDialog(WORD idDialog_, std::shared_ptr<User32Dll> User32Dll_)
	: User32DllClient(std::move(User32Dll_))
	, _idDialog(idDialog_)
{
}

/*!
 * モーダルダイアログを表示します。
 */
INT_PTR CCustomDialog::Box(HINSTANCE hLangRsrcInstance, HWND hWndParent)
{
	return GetUser32Dll()->DialogBoxParamW(
		hLangRsrcInstance,
		MAKEINTRESOURCEW(_idDialog),
		hWndParent,
		DialogProc,
		std::bit_cast<LPARAM>(this));
}

/*!
 * ダイアログをウインドウとして表示します。
 */
HWND CCustomDialog::Create(HINSTANCE hLangRsrcInstance, HWND hWndParent)
{
	const auto hWnd = GetUser32Dll()->CreateDialogParamW(
		hLangRsrcInstance,
		MAKEINTRESOURCEW(_idDialog),
		hWndParent,
		DialogProc,
		std::bit_cast<LPARAM>(this));

	return hWnd;
}

/*!
 * ダイアログのメッセージ配送
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
INT_PTR CCustomDialog::DispatchDlgEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// ダイアログの標準プロシージャがWM_SIZEを送出してくれないので挙動を上書きする
	if (const auto lpwpos = std::bit_cast<LPWINDOWPOS>(lParam);
		uMsg == WM_WINDOWPOSCHANGED && lpwpos)
	{
		// WM_MOVEはクライアント座標の原点の絶対座標を指定して送信。
		POINT pt = {};
		ClientToScreen(hDlg, &pt);
		FORWARD_WM_MOVE(hDlg, pt.x, pt.y, SendMessageW);

		// WM_SIZEはクライアント矩形のサイズを指定して送信。
		RECT rc = {};
		GetClientRect(hDlg, &rc);
		FORWARD_WM_SIZE(hDlg, SIZE_RESTORED, rc.right - rc.left, rc.bottom - rc.top, SendMessageW);

		return TRUE;
	}

	if (uMsg == WM_COMMAND)
	{
		return OnDlgCommand(hDlg, LOWORD(wParam), std::bit_cast<HWND>(lParam), HIWORD(wParam));
	}

	// WM_INITDIALOGの戻り値は他と意味が異なるので個別に処理する
	if (uMsg == WM_INITDIALOG)
	{
		const auto ret = HANDLE_WM_INITDIALOG(hDlg, wParam, lParam, OnDlgInitDialog);

		/* ダイアログデータの設定 */
		SetDlgData(hDlg);

		return ret;
	}

	return FALSE;
}

/*!
 * WM_INITDIALOGハンドラ
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @param [in] hWndFocus フォーカスを受け取る子ウインドウのハンドル
 * @param [in] lParam ダイアログパラメーター
 * @retval TRUE  フォーカスを設定する
 * @retval FALSE フォーカスを設定しない
 */
BOOL CCustomDialog::OnDlgInitDialog(HWND hDlg, HWND hWndFocus, LPARAM lParam)
{
	return TRUE;
}

/*!
 * WM_COMMANDハンドラ。
 *
 * @retval TRUE  メッセージは処理された（≒デフォルト処理は呼び出されない。）
 * @retval FALSE メッセージは処理されなかった（≒デフォルト処理が呼び出される。）
 */
BOOL CCustomDialog::OnDlgCommand(HWND hDlg, int id, HWND hWndCtl, UINT codeNotify)
{
	if (id == IDOK || id == IDCANCEL)
	{
		GetUser32Dll()->EndDialog(hDlg, id);

		return TRUE;
	}

	return FALSE;
}

/*!
 * ダイアログにデータを反映する
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @note 反映コードでメンバーを変更してはならない
 */
void CCustomDialog::SetDlgData(HWND hDlg) const
{
	UNREFERENCED_PARAMETER(hDlg);
}

/*!
 * ダイアログからデータを取り込む
 *
 * @param [in] hDlg 宛先ウインドウのハンドル
 * @retval >  0 取り込み正常
 * @retval == 0 取り込みデータなし
 * @retval <  0 取り込み異常
 */
INT_PTR CCustomDialog::GetDlgData(HWND hDlg)
{
	UNREFERENCED_PARAMETER(hDlg);

	return 0;
}
