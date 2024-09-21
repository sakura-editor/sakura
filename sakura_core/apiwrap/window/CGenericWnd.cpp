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
#include "StdAfx.h"
#include "apiwrap/window/CGenericWnd.hpp"

namespace apiwrap::window
{

/*!
 * ウインドウを作成します。
 */
HWND CGenericWnd::CreateWindowExW(
    HWND                hWndParent,
    UINT                windowId,
    const CWndClass&    wndClass,
    DWORD               dwStyle,
    DWORD               dwExStyle,
    std::wstring_view   windowTitle,
    std::optional<RECT> rcDesired
) const
{
    RECT rcWin = { CW_USEDEFAULT, SW_SHOWDEFAULT, CW_USEDEFAULT, 0 };
    if (rcDesired.has_value())
    {
        rcWin = rcDesired.value();
    }

	const auto hWnd = ::CreateWindowExW(
        dwExStyle,
        wndClass.GetClassNameW(),
        windowTitle.data(),
        dwStyle,
        rcWin.left,
        rcWin.top,
        rcWin.right - rcWin.left,
        rcWin.bottom - rcWin.top,
        hWndParent,
        HMENU(size_t(windowId)),
        wndClass.GetInstance(),
        LPVOID(this)
	);

	return hWnd;
}

} // end of namespace apiwrap::window
