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
#include "apiwrap/window/COriginalWnd.hpp"

namespace apiwrap::window
{

/*!
 * WndProc(独自ウインドウのメッセージ配送)
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT CALLBACK COriginalWnd::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // SetWindowLongPtr/GetWindowLongPtrの引数にNULLはマズい
    if (!hWnd || !::IsWindow(hWnd))
    {
        return 0L;
    }

    // WM_NCCREATEが来たらウインドウハンドルにクラスインスタンスを紐付ける
    if (const auto lpCreateStruct = LPCREATESTRUCT(lParam);
        uMsg == WM_NCCREATE && lpCreateStruct && lpCreateStruct->lpCreateParams)
    {
        // 作成パラメーターからインスタンスを取り出す
        auto pcWnd = static_cast<Me*>(lpCreateStruct->lpCreateParams);

        // 取り出したインスタンスをユーザーデータに登録する
        ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, LONG_PTR(pcWnd));

        // ウインドウハンドルをインスタンスに紐付ける
        pcWnd->_SetHwnd(hWnd);
    }

    // ユーザーデータに登録されたインスタンスを取り出す
    if (auto pcWnd = CGenericWnd::FromHwnd<Me>(hWnd))
    {
        // インスタンスのメッセージ配送を呼び出す
        const auto ret = pcWnd->DispatchEvent(hWnd, uMsg, wParam, lParam);

        // WM_NCDESTROYが来たらウインドウハンドルとインスタンスの紐付けを解除する
        if (WM_NCDESTROY == uMsg)
        {
            // ウインドウハンドルの紐付けを解除する
            pcWnd->_SetHwnd(nullptr);

            // ユーザーデータの登録を解除する
            ::SetWindowLongPtrW(hWnd, GWLP_USERDATA, 0);
        }

        return ret;
    }

    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

/*!
 * コンストラクタ
 */
COriginalWnd::COriginalWnd(std::wstring_view className, HINSTANCE hInstance, int cbWndExtra, int cbClsExtra) noexcept
    : m_hInstance(hInstance)
    , m_WndClass(className, m_hInstance, &COriginalWnd::WndProc, cbWndExtra, cbClsExtra)
{
}

/*!
 * ウインドウクラスを登録します。
 */
bool COriginalWnd::RegisterWnd(
    HCURSOR hCursor,
    HBRUSH  hbrBackground,
    UINT    uStyles,
    HICON   hIcon,
    HICON   hIconSm,
    LPCWSTR lpszMenuName
) noexcept
{
    return m_WndClass.RegisterWndClass(hCursor, hbrBackground, uStyles, hIcon, hIconSm, lpszMenuName);
}


/*!
 * ウインドウを作成します。
 */
HWND COriginalWnd::CreateWnd(
    HWND                    hWndParent,
    UINT                    windowId,
    DWORD                   dwStyle,
    DWORD                   dwExStyle,
    std::wstring_view       windowTitle,
    std::optional<RECT>     rcDesired
) noexcept
{
    return CreateWindowExW(
		hWndParent,
		windowId,
		m_WndClass,
		dwStyle,
        dwExStyle,
		windowTitle,
        rcDesired
    );
}

/*!
 * ウインドウを作成します。
 */
HWND COriginalWnd::CreateWnd(
    const CGenericWnd*      pParentWnd,
    UINT                    windowId,
    DWORD                   dwStyle,
    DWORD                   dwExStyle,
    std::wstring_view       windowTitle,
    std::optional<RECT>     rcDesired
) noexcept
{
    return CreateWnd(
        pParentWnd ? pParentWnd->GetHwnd() : nullptr,
		windowId,
		dwStyle,
        dwExStyle,
		windowTitle,
        rcDesired
    );
}

/*!
 * 独自ウインドウのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT COriginalWnd::DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (WM_CREATE == uMsg)
    {
        return HANDLE_WM_CREATE(hWnd, wParam, lParam, OnCreate);
    }

    return DefWindowProcW(hWnd, uMsg, wParam, lParam);
}

/*!
 * デフォルトのメッセージ配送
 *
 * @param [in] hWnd 宛先ウインドウのハンドル
 * @param [in] uMsg メッセージコード
 * @param [in, opt] wParam 第1パラメーター
 * @param [in, opt] lParam 第2パラメーター
 * @returns 処理結果 メッセージコードにより異なる
 */
LRESULT COriginalWnd::DefWindowProcW(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) const
{
    return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
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
bool COriginalWnd::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    if (!hWnd)
    {
        return false;
    }

    if (!lpCreateStruct)
    {
        return false;
    }

    m_hwndParent = lpCreateStruct->hwndParent;

    return true;
}

} // end of namespace apiwrap::window
