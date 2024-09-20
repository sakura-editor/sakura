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

class CWndClass
{
private:
    HINSTANCE       m_hInstance;        //!< モジュールのインスタンスハンドル
    std::wstring    m_ClassName;        //!< ウインドウクラス名
    WNDPROC         m_WndProc;          //!< ウインドウプロシージャ
    int             m_cbWndExtra;       //!< ウインドウデータの拡張領域サイズ(byte単位)
    int             m_cbClsExtra;       //!< クラスデータの拡張領域サイズ(byte単位)
    ATOM            m_ClassAtom = 0;    //!< ウインドウクラスアトム

public:
    /*!
     * コンストラクタ
     */
    explicit CWndClass(
        std::wstring_view   className,
        HINSTANCE           hInstance   = nullptr,
        const WNDPROC&      lpfnWndProc = nullptr,
        int                 cbWndExtra  = 0,
        int                 cbClsExtra  = 0
    ) noexcept;

    /*!
     * ウインドウクラスを登録します。
     */
    bool RegisterWndClass(
        HCURSOR hCursor       = HCURSOR(nullptr),
        HBRUSH  hbrBackground = HBRUSH(COLOR_WINDOW + 1),
        UINT    uStyles       = CS_HREDRAW | CS_VREDRAW,
        HICON   hIcon         = HICON(nullptr),
        HICON   hIconSm       = HICON(nullptr),
        LPCWSTR lpszMenuName  = LPCWSTR(nullptr)
    ) noexcept;

    HINSTANCE         GetInstance() const noexcept;
    LPCWSTR           GetClassNameW() const noexcept;
    std::wstring_view GetOriginalClassName() const noexcept;
};

} // end of namespace apiwrap::window
