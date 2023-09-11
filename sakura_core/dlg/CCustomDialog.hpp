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
#pragma once

#include "apimodule/User32Dll.hpp"

#include "apiwrap/apiwrap.hpp"

#include <windowsx.h>

/*!
 * カスタムダイアログクラス
 * 
 * すべてのダイアログはカスタム。
 */
class CCustomDialog : public User32DllClient
{
	using Me = CCustomDialog;

public:
	WORD            _idDialog = 0L;
	HWND			m_hWnd    = nullptr;

	explicit CCustomDialog(WORD idDialog_ = 0, std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>());
	~CCustomDialog() override = default;

	virtual INT_PTR Box(HINSTANCE hLangRsrcInstance, HWND hWndParent = NULL);
	virtual HWND    Create(HINSTANCE hLangRsrcInstance, HWND hWndParent = NULL);

	/*!
	 * ダイアログをウインドウとして表示します。
	 */
	template<typename TFunc>
	HWND CreateIndirect(HINSTANCE hLangRsrcInstance, const TFunc& func, HWND hWndParent)
	{
		auto buffer = apiwrap::CopyResource<LPDLGTEMPLATE>(hLangRsrcInstance, MAKEINTRESOURCE(_idDialog), RT_DIALOG, GetUser32Dll());
		if (buffer.empty())
		{
			return nullptr;
		}

		auto lpDlgTemplate = std::bit_cast<LPDLGTEMPLATE>(buffer.data());
		func(*lpDlgTemplate);

		const auto hWnd = GetUser32Dll()->CreateDialogIndirectParamW(
			hLangRsrcInstance,
			lpDlgTemplate,
			hWndParent,
			DialogProc,
			std::bit_cast<LPARAM>(this)
		);

		return hWnd;
	}

	HWND    GetHwnd() const noexcept { return m_hWnd; }

private:
	static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	virtual INT_PTR DispatchDlgEvent(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

	virtual BOOL    OnDlgInitDialog(HWND hDlg, HWND hwndFocus, LPARAM lParam);
	virtual BOOL    OnDlgCommand(HWND hDlg, int id, HWND hWndCtl, UINT codeNotify);

	virtual void    SetDlgData(HWND hDlg) const;
	virtual INT_PTR GetDlgData(HWND hDlg);
};
