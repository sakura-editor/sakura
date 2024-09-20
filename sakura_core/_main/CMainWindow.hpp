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

#include "apiwrap/window/COriginalWnd.hpp"
#include "env/SShareDataClientWithCache.hpp"

#include "uiparts/CMenuDrawer.h"
#include "uiparts/CImageListMgr.h" // 2002/2/10 aroka
#include "CPropertyManager.h"

/*!
 * メインウインドウ
 */
class CMainWindow : public apiwrap::window::COriginalWnd, public SShareDataClientWithCache
{
private:
    using Me = CMainWindow;
	using CPropManagerHolder = std::shared_ptr<CPropertyManager>;

public:
	using COriginalWnd::COriginalWnd;
	~CMainWindow() override = default;

	virtual HWND    CreateMainWnd(int nCmdShow) = 0;
	virtual void    MessageLoop(void) = 0;

	bool    OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct) override;

	CImageListMgr       m_hIcons;
	CMenuDrawer         m_cMenuDrawer;
	CPropManagerHolder  m_pcPropertyManager = std::make_shared<CPropertyManager>();
};
