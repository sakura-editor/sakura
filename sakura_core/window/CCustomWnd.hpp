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
#include "window/CGenericWnd.hpp"

/*!
 * カスタムウインドウ
 * 
 * 独自に定義するウインドウのためのクラスです。
 */
class CCustomWnd : public CGenericWnd, public User32DllClient
{
private:
    using Me = CCustomWnd;

    /*!
     * ウインドウクラス名
     * 
     * RegisterWndメソッドが成功すると、このメンバーにコピーされます。
     */
    std::wstring _ClassName;

public:
    explicit CCustomWnd(std::shared_ptr<User32Dll> User32Dll_ = std::make_shared<User32Dll>()) noexcept;
    ~CCustomWnd() override = default;

    bool RegisterWnd(
        std::wstring_view className,
        HCURSOR           hCursor       = static_cast<HCURSOR>(nullptr),
        HBRUSH            hbrBackground = static_cast<HBRUSH>(nullptr),
        UINT              uStyles       = CS_HREDRAW | CS_VREDRAW,
        HICON             hIcon         = static_cast<HICON>(nullptr),
        HICON             hIconSm       = static_cast<HICON>(nullptr),
        int               cbWndExtra    = 0);

    HWND CreateWnd(
        HWND              hWndParent,
        std::wstring_view windowTitle  = L"",
        DWORD             dwStyle      = WS_OVERLAPPEDWINDOW,
        DWORD             dwExStyle    = 0L,
        UINT              windowId     = 0L,
        const RECT*       prcWin       = nullptr);

protected:
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    virtual bool    OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);

    LRESULT DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const override;
};
