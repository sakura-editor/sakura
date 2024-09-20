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
#include "_main/CMainWindow.hpp"

#include "CSelectLang.h"

 /*!
 * WM_CREATEハンドラ
 *
 * WM_CREATEはCreateWindowEx関数によるウインドウ作成中にポストされます。
 * メッセージの戻り値はウインドウの作成を続行するかどうかの判断に使われます。
 *
 * @retval true  ウィンドウの作成を続行する
 * @retval false ウィンドウの作成を中止する
 */
bool CMainWindow::OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
    if (!__super::OnCreate(hWnd, lpCreateStruct))
    {
        return false;
    }

	m_hIcons.Create(m_hInstance);
	m_cMenuDrawer.Create(CSelectLang::getLangRsrcInstance(), hWnd, &m_hIcons);
	m_pcPropertyManager->Create(hWnd, &m_hIcons, &m_cMenuDrawer );

	return true;
}
