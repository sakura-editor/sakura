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

	virtual BOOL CloseClipboard() const
	{
		return ::CloseClipboard();
	}

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

	virtual HWND CreateWindowExW(
		_In_ DWORD dwExStyle,
		_In_opt_ LPCWSTR lpClassName,
		_In_opt_ LPCWSTR lpWindowName,
		_In_ DWORD dwStyle,
		_In_ int X,
		_In_ int Y,
		_In_ int nWidth,
		_In_ int nHeight,
		_In_opt_ HWND hWndParent,
		_In_opt_ HMENU hMenu,
		_In_opt_ HINSTANCE hInstance,
		_In_opt_ LPVOID lpParam) const
	{
		return ::CreateWindowExW(dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam);
	}

	virtual LRESULT DefWindowProcW(
		_In_ HWND hWnd,
		_In_ UINT uMsg,
		_In_ WPARAM wParam,
		_In_ LPARAM lParam) const
	{
		return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
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

	virtual BOOL EmptyClipboard() const
	{
		return ::EmptyClipboard();
	}

	virtual BOOL EndDialog(
			_In_ HWND hDlg,
			_In_ INT_PTR nResult) const
	{
		return ::EndDialog(hDlg, nResult);
	}

	virtual UINT EnumClipboardFormats(
		_In_ UINT format) const
	{
		return ::EnumClipboardFormats(format);
	}

	virtual HRSRC FindResourceW(
		_In_opt_ HMODULE hModule,
		_In_ LPCWSTR lpName,
		_In_ LPCWSTR lpType) const
	{
		return ::FindResourceW(hModule, lpName, lpType);
	}

	virtual BOOL GetClassInfoExW(
		_In_opt_ HINSTANCE hInstance,
		_In_ LPCWSTR lpszClass,
		_Out_ LPWNDCLASSEXW lpwcx) const
	{
		return ::GetClassInfoExW(hInstance, lpszClass, lpwcx);
	}

	virtual HANDLE GetClipboardData(
		_In_ UINT uFormat) const
	{
		return ::GetClipboardData(uFormat);
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

	virtual BOOL IsClipboardFormatAvailable(
		_In_ UINT format) const
	{
		return ::IsClipboardFormatAvailable(format);
	}

	virtual HGLOBAL LoadResource(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo) const
	{
		return ::LoadResource(hModule, hResInfo);
	}

	virtual LPVOID LockResource(
		_In_ HGLOBAL hResData) const
	{
		return ::LockResource(hResData);
	}

	virtual BOOL OpenClipboard(
		_In_opt_ HWND hWndNewOwner) const
	{
		return ::OpenClipboard(hWndNewOwner);
	}

	virtual ATOM RegisterClassExW(
		_In_ CONST WNDCLASSEXW* lpwcex) const
	{
		return ::RegisterClassExW(lpwcex);
	}

	virtual UINT RegisterClipboardFormatW(
		_In_ LPCWSTR lpszFormat) const
	{
		return ::RegisterClipboardFormatW(lpszFormat);
	}

	virtual LRESULT SendMessageW(
		_In_ HWND hWnd,
		_In_ UINT Msg,
		_Pre_maybenull_ _Post_valid_ WPARAM wParam,
		_Pre_maybenull_ _Post_valid_ LPARAM lParam) const
	{
		return ::SendMessageW(hWnd, Msg, wParam, lParam);
	}

	virtual HANDLE SetClipboardData(
		_In_ UINT uFormat,
		_In_opt_ HANDLE hMem) const
	{
		return ::SetClipboardData(uFormat, hMem);
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

	virtual DWORD SizeofResource(
		_In_opt_ HMODULE hModule,
		_In_ HRSRC hResInfo) const
	{
		return ::SizeofResource(hModule, hResInfo);
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
