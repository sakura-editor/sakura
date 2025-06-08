/*!	@file
	@brief HtmpHelp動的ロード
	
	HTML Help コンポーネントへの動的アクセスクラス

	@author genta
	@date Jul. 5, 2001
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/

#include "StdAfx.h"
#include "CHtmlHelp.h"

CHtmlHelp::CHtmlHelp(void) noexcept
	: m_pfnHtmlHelp(nullptr)
{
}

/*!
	HTML Help のファイル名を渡す
*/
LPCWSTR CHtmlHelp::GetDllNameImp(int nIndex)
{
	return L"HHCTRL.OCX";
}

bool CHtmlHelp::InitDllImp()
{
	//DLL内関数名リスト
	const ImportTable table[] = {
		{ (void*)&m_pfnHtmlHelp,		"HtmlHelpW" },
		{ nullptr, nullptr }
	};

	if (!RegisterEntries(table)) {
		return false;
	}

	return true;
}
