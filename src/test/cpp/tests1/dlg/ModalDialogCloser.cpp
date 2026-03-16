/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "dlg/ModalDialogCloser.hpp"

namespace dialog {

/*!
 * CBTフックプロシージャ
 *
 * CBT(Computer-Based Training)フックを使ってウインドウイベントを捕捉する。
 */
LRESULT CALLBACK ModalDialogCloser::CBTProc(
  _In_ int    nCode,
  _In_ WPARAM wParam,
  _In_ LPARAM lParam
)
{
	static HWND hWnd = nullptr;

	// ダイアログボックスの作成イベントを捕捉する
	if (const auto pCreateWnd = LPCBT_CREATEWND(lParam);
		HCBT_CREATEWND == nCode &&
		pCreateWnd &&
		pCreateWnd->lpcs &&
		IS_INTRESOURCE(pCreateWnd->lpcs->lpszClass) &&
		DIALOG_CLASS == LOWORD(pCreateWnd->lpcs->lpszClass))
	{
		hWnd = std::bit_cast<HWND>(wParam);
	}
	else if (HCBT_ACTIVATE == nCode && hWnd == std::bit_cast<HWND>(wParam)) {
		::EndDialog(hWnd, FALSE); // サクラエディタの独自仕様で「キャンセルで閉じる」の意。
	}
	else if (HCBT_DESTROYWND == nCode && hWnd == std::bit_cast<HWND>(wParam)) {
		hWnd = nullptr;
	}

	return ::CallNextHookEx(gm_CbtHook, nCode, wParam, lParam);
}

ModalDialogCloser::ModalDialogCloser() noexcept
{
	gm_CbtHook = ::SetWindowsHookExW(WH_CBT, &CBTProc, nullptr, ::GetCurrentThreadId());
}

ModalDialogCloser::~ModalDialogCloser() noexcept
{
	gm_CbtHook = nullptr;
}

} // namespace dialog
