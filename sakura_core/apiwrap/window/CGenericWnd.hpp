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

namespace apiwrap::window
{

/*!
 * 汎用ウインドウ
 * 
 * どんなウインドウにも適用できる汎用のウインドウクラスです。
 */
class CGenericWnd
{
private:
    using Me = CGenericWnd;

public:
    HWND    m_hWnd = nullptr;

    virtual ~CGenericWnd() = default;

    HWND    GetHwnd() const noexcept { return m_hWnd; }

protected:
    void    _SetHwnd(_In_opt_ HWND hWnd) noexcept { m_hWnd = hWnd; }

    /*!
     * ウインドウのメッセージ配送
     *
     * @param [in] hWnd 宛先ウインドウのハンドル
     * @param [in] uMsg メッセージコード
     * @param [in, opt] wParam 第1パラメーター
     * @param [in, opt] lParam 第2パラメーター
     * @returns 処理結果 メッセージコードにより異なる
     */
    virtual LRESULT DispatchEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
};

} // end of namespace apiwrap::window
