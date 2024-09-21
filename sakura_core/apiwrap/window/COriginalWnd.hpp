/*! @file */
/*
    Copyright (C) 2024, Sakura Editor Organization

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

#include "apiwrap/window/CGenericWnd.hpp"
#include "apiwrap/window/CWndClass.hpp"

namespace apiwrap::window
{

/*!
 * 独自ウインドウ
 * 
 * 独自定義のウインドウクラスです。
 */
class COriginalWnd : public apiwrap::window::CGenericWnd
{
private:
    using Me = COriginalWnd;

public:
    HINSTANCE   m_hInstance;
    
    CWndClass   m_WndClass;

	HWND        m_hwndParent    = nullptr;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    explicit COriginalWnd(std::wstring_view className, HINSTANCE hInstance = GetModuleHandleW(nullptr), int cbWndExtra = 0, int cbClsExtra = 0) noexcept;

    std::wstring_view   GetClassNameW() const noexcept { return m_WndClass.GetOriginalClassName(); }
    HWND                GetHwndParent() const noexcept { return m_hwndParent; }

    /*!
     * ウインドウクラスを登録します。
     */
    virtual bool RegisterWnd(
        HCURSOR hCursor       = HCURSOR(nullptr),
        HBRUSH  hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        UINT    uStyles       = CS_HREDRAW | CS_VREDRAW,
        HICON   hIcon         = HICON(nullptr),
        HICON   hIconSm       = HICON(nullptr),
        LPCWSTR lpszMenuName  = LPCWSTR(nullptr)
    ) noexcept;

    /*!
     * ウインドウを作成します。
     */
    HWND CreateWnd(
        HWND                    hWndParent,
        UINT                    windowId    = 0L,
        DWORD                   dwStyle     = WS_OVERLAPPED,
        DWORD                   dwExStyle   = 0L,
        std::wstring_view       windowTitle = L""sv,
        std::optional<RECT>     rcDesired   = std::nullopt
    ) noexcept;

    /*!
     * ウインドウを作成します。
     */
    HWND CreateWnd(
        const CGenericWnd*      pParentWnd,
        UINT                    windowId    = 0L,
        DWORD                   dwStyle     = WS_OVERLAPPED,
        DWORD                   dwExStyle   = 0L,
        std::wstring_view       windowTitle = L""sv,
        std::optional<RECT>     rcDesired   = std::nullopt
    ) noexcept;

    LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

    virtual bool    OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct);

    virtual LRESULT DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const;
};

} // end of namespace apiwrap::window
