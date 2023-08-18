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
#include "window/CCustomWnd.hpp"

#include "apiwrap/cstring.hpp"

/*!
 * WndProc(カスタムウインドウのメッセージ配送)
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CALLBACK CCustomWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// SetWindowLongPtr/GetWindowLongPtrの引数にNULLはマズい
	if (!hWnd)
	{
		return 0L;
	}

	// WM_NCCREATEが来たらウインドウハンドルにクラスインスタンスを紐付ける
	if (const auto lpCreateStruct = std::bit_cast<LPCREATESTRUCT>(lParam);
		uMsg == WM_NCCREATE && lpCreateStruct && lpCreateStruct->lpCreateParams)
	{
		auto pcWnd = static_cast<Me*>(lpCreateStruct->lpCreateParams);

		::SetWindowLongPtrW(hWnd, GWLP_USERDATA, std::bit_cast<LONG_PTR>(pcWnd));

		pcWnd->m_hWnd = hWnd;
	}

	// GetWindowLongPtrでインスタンスを取り出し、処理させる
	if (auto pcWnd = std::bit_cast<Me*>(::GetWindowLongPtrW(hWnd, GWLP_USERDATA)))
	{
		// WM_NCDESTROYが来たらウインドウハンドルとインスタンスの紐付けを解除する
		if (uMsg == WM_NCDESTROY)
		{
			pcWnd->m_hWnd = nullptr;

			::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);

			return 0L;
		}

		return pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

/*!
 * コンストラクタ
 */
CCustomWnd::CCustomWnd(std::shared_ptr<User32Dll> User32Dll_) noexcept
	: User32DllClient(std::move(User32Dll_))
{
}

/*!
 * ウインドウクラスを登録します。
 */
bool CCustomWnd::RegisterWnd(
	std::wstring_view className,
	HCURSOR           hCursor,
	HBRUSH            hbrBackground,
	UINT              uStyles,
	HICON             hIcon,
	HICON             hIconSm,
	int               cbWndExtra)
{
	if (className.empty())
	{
		return false;
	}

	apiwrap::cstring buff1(className);
	className = static_cast<std::wstring_view>(buff1);

	if (WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), 0 };
		!apiwrap::IsWndClassRegistered(className, GetUser32Dll()))
	{
        wc.style         = uStyles;
        wc.lpfnWndProc   = WndProc;
        wc.cbClsExtra    = 0;
        wc.cbWndExtra    = cbWndExtra;
        wc.hInstance     = nullptr;
        wc.hIcon         = hIcon;
        wc.hCursor       = hCursor;
        wc.hbrBackground = hbrBackground;
        wc.lpszMenuName  = nullptr;
        wc.lpszClassName = className.data();
        wc.hIconSm       = hIconSm;

		if (!GetUser32Dll()->RegisterClassExW(&wc))
		{
			return false;
		}
	}

	_ClassName = className;

	return true;
}

/*!
 * ウインドウを作成します。
 */
HWND CCustomWnd::CreateWnd(
	HWND              hWndParent,
	std::wstring_view windowTitle,
	DWORD             dwStyle,
	DWORD             dwExStyle,
	UINT              windowId,
	const RECT*       prcWin)
{
	if (_ClassName.empty())
	{
		return NULL;
	}

	apiwrap::cstring buff1(windowTitle);
	windowTitle = static_cast<std::wstring_view>(buff1);

	RECT rcWin = { CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT };
	if (prcWin)
	{
        rcWin.left   = prcWin->left;
        rcWin.top    = prcWin->top;
        rcWin.right  = prcWin->right;
        rcWin.bottom = prcWin->bottom;
	}

	const auto hWnd = GetUser32Dll()->CreateWindowExW(
		dwExStyle,
		_ClassName.data(),
		windowTitle.data(),
		dwStyle,
		rcWin.left,
		rcWin.top,
		rcWin.right,
		rcWin.bottom,
		hWndParent,
		std::bit_cast<HMENU>(static_cast<size_t>(windowId)),
		static_cast<HINSTANCE>(NULL),
		static_cast<LPVOID>(this));

	return hWnd;
}

/*!
 * カスタムウインドウのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CCustomWnd::DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CREATE) {
		return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
    }

	return __super::DispatchEvent(hWnd, uMsg, wParam, lParam);
}

/*!
 * WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 * メッセージの戻り値はウインドウの作成を続行するかどうかの判断に使われます。
 * 
 * @retval true  ウィンドウの作成を続行する
 * @retval false ウィンドウの作成を中止する
 */
bool CCustomWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	if (!hWnd)
	{
		return false;
	}

	if (!lpCreateStruct)
	{
		return false;
	}

	return true;
}

/*!
 * デフォルトメッセージハンドラ
 *
 * User32.dllのDefWindowProcWに処理を委譲します。
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CCustomWnd::DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
	return GetUser32Dll()->DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
