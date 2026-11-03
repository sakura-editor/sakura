/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#pragma once

#include "env/CSakuraEnvironment.h"	//env::ShareDataClient
#include "env/CPropertyManager.h"
#include "uiparts/CImageListMgr.h" // 2002/2/10 aroka
#include "uiparts/CMenuDrawer.h"

#include <memory>

#include <Windows.h>

enum class EAppExitCode : int;

/*!
 * @brief アプリのメインウィンドウ
 *
 * アプリのメインウィンドウに共通する変数・処理を抜き出して定義する。
 */
class CAppMainWnd : public env::ShareDataClient
{
private:
	using PropMgrHolder = std::unique_ptr<CPropertyManager>;

	using Me = CAppMainWnd;

public:
	explicit CAppMainWnd(
		_In_ HINSTANCE hInstance
	);

	virtual ~CAppMainWnd() = default;

	virtual EAppExitCode MessageLoop() const = 0;

	HINSTANCE		GetAppInstance() const noexcept { return m_hInstance; }

	HWND			GetHwnd() const noexcept { return m_hWnd; }

	CImageListMgr&	GetIcons() noexcept { return m_hIcons; }

	CMenuDrawer&	GetMenuDrawer() noexcept { return m_cMenuDrawer; }

	HINSTANCE		m_hInstance;
	HWND			m_hWnd = nullptr;

	CImageListMgr	m_hIcons;

	CMenuDrawer		m_cMenuDrawer;

	PropMgrHolder	m_pcPropertyManager = std::make_unique<CPropertyManager>();
};
