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
#include "apiwrap/window/CWndClass.hpp"

namespace apiwrap::window
{

/*!
 * コンストラクタ
 */
CWndClass::CWndClass(
	std::wstring_view   className,
    HINSTANCE           hInstance,
    const WNDPROC&      lpfnWndProc,
    int                 cbWndExtra,
    int                 cbClsExtra
) noexcept
    : m_hInstance(hInstance)
    , m_ClassName(className)
    , m_WndProc(lpfnWndProc)
    , m_cbWndExtra(cbWndExtra)
    , m_cbClsExtra(cbClsExtra)
{
}

/*!
 * ウインドウクラスを登録します。
 */
bool CWndClass::RegisterWndClass(
    HCURSOR hCursor,
    HBRUSH  hbrBackground,
    UINT    uStyles,
    HICON   hIcon,
    HICON   hIconSm,
    LPCWSTR lpszMenuName
) noexcept
{
	// 登録不可 or 登録不要
    if (m_ClassName.empty() || !m_WndProc)
    {
        return false;
    }

	// クラスアトム未設定
	if (!m_ClassAtom)
    {
		// 登録済み情報を照会
		WNDCLASSEXW wc = { sizeof(WNDCLASSEXW), 0 };
		m_ClassAtom = (ATOM)GetClassInfoExW(m_hInstance, m_ClassName.data(), &wc);

		// 未登録
		if (!m_ClassAtom)
		{
			wc.style         = uStyles;
			wc.lpfnWndProc   = m_WndProc;
			wc.cbClsExtra    = m_cbClsExtra;
			wc.cbWndExtra    = m_cbWndExtra;
			wc.hInstance     = m_hInstance;
			wc.hIcon         = hIcon;
			wc.hCursor       = hCursor;
			wc.hbrBackground = hbrBackground;
			wc.lpszMenuName  = lpszMenuName;
			wc.lpszClassName = m_ClassName.data();
			wc.hIconSm       = hIconSm;

			m_ClassAtom = RegisterClassExW(&wc);
		}
	}

	return m_ClassAtom;
}

HINSTANCE CWndClass::GetInstance() const noexcept
{
    return m_hInstance;
}

LPCWSTR CWndClass::GetClassNameW() const noexcept
{
    return m_ClassAtom ? MAKEINTRESOURCEW(m_ClassAtom) : m_ClassName.data();
}

std::wstring_view CWndClass::GetOriginalClassName() const noexcept
{
    return m_ClassName;
}

} // end of namespace apiwrap::window
