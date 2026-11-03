/*! @file */
/*
	Copyright (C) 2026, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
 */
#include "StdAfx.h"
#include "window/CAppMainWnd.hpp"

#include "CSelectLang.h"

/*!
 * @brief コンストラクタ
 *
 * @param[in] hInstance インスタンスハンドル
 */
CAppMainWnd::CAppMainWnd(
	_In_ HINSTANCE hInstance
)
	: m_hInstance(hInstance)
{
	m_hIcons.Create(m_hInstance);

	m_cMenuDrawer.Create(
		CSelectLang::getLangRsrcInstance(),
		GetHwnd(),	// 常にNULLなので、指定する意味はない。
		&GetIcons()
	);
}
