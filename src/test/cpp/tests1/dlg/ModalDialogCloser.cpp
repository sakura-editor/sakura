/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "pch.h"
#include "dlg/ModalDialogCloser.hpp"

#include "doc/CDocListener.h"

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
		auto pThis = getInstance();
		pThis->m_Action(hWnd);
	}
	else if (HCBT_DESTROYWND == nCode && hWnd == std::bit_cast<HWND>(wParam)) {
		hWnd = nullptr;
	}

	return ::CallNextHookEx(gm_CbtHook, nCode, wParam, lParam);
}

/* static */ void ModalDialogCloser::CloseDialogByCancel(HWND hWndDlg)
{
	::SendMessageW(hWndDlg, WM_COMMAND, MAKELONG(IDCANCEL, BN_CLICKED), 0);
}

ModalDialogCloser::ModalDialogCloser(const std::function<void(HWND)>& action) noexcept
	: m_Action(action)
{
	gm_CbtHook = ::SetWindowsHookExW(WH_CBT, &CBTProc, nullptr, ::GetCurrentThreadId());
}

ModalDialogCloser::ModalDialogCloser() noexcept
	: ModalDialogCloser(&CloseDialogByCancel)
{
}

ModalDialogCloser::~ModalDialogCloser() noexcept
{
	gm_CbtHook = nullptr;
}

} // namespace dialog


/* static */ void MockCDlgOpenFile::_Cleanup([[maybe_unused]] const MockCDlgOpenFile*)
{
	gm_Files.clear();
}

/* static */ bool MockCDlgOpenFile::_GetOpenFileName(std::span<WCHAR> szPath, EFilter eAddFileter)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(std::data(szPath), std::size(szPath), std::data(gm_Files.front()));
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_GetSaveFileName(std::span<WCHAR> szPath)
{
	const auto result = !gm_Files.empty();
	if (result) {
		::wcscpy_s(std::data(szPath), std::size(szPath), std::data(gm_Files.front()));
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_DoModalOpenDlg(
	SLoadInfo* pLoadInfo,
	std::vector<std::wstring>* pFilenames,
	bool bOptions [[maybe_unused]]
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		pLoadInfo->cFilePath = gm_Files.front().c_str();
		*pFilenames = gm_Files;
	}
	return result;
}

/* static */ bool MockCDlgOpenFile::_DoModalSaveDlg(
	SSaveInfo* pSaveInfo,
	bool bSimpleMode [[maybe_unused]]
)
{
	const auto result = !gm_Files.empty();
	if (result) {
		pSaveInfo->cFilePath = gm_Files.front().c_str();
	}
	return result;
}

MockCDlgOpenFile::MockCDlgOpenFile()
{
	ON_CALL(*this, DoModal_GetOpenFileName(_, _)).WillByDefault(&_GetOpenFileName);
	ON_CALL(*this, DoModal_GetSaveFileName(_)).WillByDefault(&_GetSaveFileName);
	ON_CALL(*this, DoModalOpenDlg(_, _, _)).WillByDefault(&_DoModalOpenDlg);
	ON_CALL(*this, DoModalSaveDlg(_, _)).WillByDefault(&_DoModalSaveDlg);
}
