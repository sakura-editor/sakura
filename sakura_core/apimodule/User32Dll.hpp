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

#include <Windows.h>

#include <memory>
 
 /*!
 * User32.DLLへの依存を抽象化するクラス
 *
 * Win32 API呼出のテストを可能にする
 *
 * * 必要な定義を Windows.h からコピペする
 * * （定義順はアルファベット順、意味的なまとまりは考慮しない。）
 * * 関数定義を virtual const に変える
 * * 関数本体でグローバルの同名関数に処理を委譲させる
 */
struct User32Dll
{
	virtual ~User32Dll() = default;

	virtual HWND CreateDialogIndirectParamW(
		_In_opt_ HINSTANCE hInstance,
		_In_ LPCDLGTEMPLATEW lpTemplate,
		_In_opt_ HWND hWndParent,
		_In_opt_ DLGPROC lpDialogFunc,
		_In_ LPARAM dwInitParam) const
	{
		return ::CreateDialogIndirectParamW(hInstance, lpTemplate, hWndParent, lpDialogFunc, dwInitParam);
	}

	virtual HWND CreateDialogParamW(
		_In_opt_ HINSTANCE hInstance,
		_In_ LPCWSTR       lpTemplateName,
		_In_opt_ HWND      hWndParent,
		_In_opt_ DLGPROC   lpDialogFunc,
		_In_ LPARAM        dwInitParam) const
	{
		return ::CreateDialogParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	}

	virtual INT_PTR DialogBoxParamW(
		_In_opt_ HINSTANCE hInstance,
		_In_ LPCWSTR       lpTemplateName,
		_In_opt_ HWND      hWndParent,
		_In_opt_ DLGPROC   lpDialogFunc,
		_In_ LPARAM        dwInitParam) const
	{
		return ::DialogBoxParamW(hInstance, lpTemplateName, hWndParent, lpDialogFunc, dwInitParam);
	}

	virtual HWND GetDlgItem(
		_In_opt_ HWND hDlg,
		_In_ int nIDDlgItem) const
	{
		return ::GetDlgItem(hDlg, nIDDlgItem);
	}

	virtual int GetWindowTextW(
		_In_ HWND hWnd,
		_Out_writes_(nMaxCount) LPWSTR lpString,
		_In_ int nMaxCount) const
	{
		return ::GetWindowTextW(hWnd, lpString, nMaxCount);
	}

	virtual int GetWindowTextLengthW(
		_In_ HWND hWnd) const
	{
		return ::GetWindowTextLengthW(hWnd);
	}

	virtual LRESULT SendMessageW(
		_In_ HWND hWnd,
		_In_ UINT Msg,
		_Pre_maybenull_ _Post_valid_ WPARAM wParam,
		_Pre_maybenull_ _Post_valid_ LPARAM lParam) const
	{
		return ::SendMessageW(hWnd, Msg, wParam, lParam);
	}

	virtual LONG_PTR SetWindowLongPtrW(_In_ HWND hWnd, int nIndex, LONG_PTR dwNewLong) const
	{
		return ::SetWindowLongPtrW(hWnd, nIndex, dwNewLong);
	}

	virtual bool SetWindowTextW(
		_In_ HWND hWnd,
		_In_opt_ LPCWSTR lpString) const
	{
		return ::SetWindowTextW(hWnd, lpString);
	}

	virtual bool ShowWindow(
		_In_ HWND hWnd,
		_In_ int nCmdShow) const
	{
		return ::ShowWindow(hWnd, nCmdShow);
	}
};

class User32DllClient
{
	std::shared_ptr<User32Dll> _User32Dll;

public:
	explicit User32DllClient(std::shared_ptr<User32Dll> User32Dll_)
		: _User32Dll(std::move(User32Dll_))
	{
	}
	virtual ~User32DllClient() = default;

protected:
	std::shared_ptr<User32Dll> GetUser32Dll() const noexcept
	{
		return _User32Dll;
	}
};
