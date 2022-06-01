﻿/*!	@file
	@brief HtmpHelp動的ロード

	HTML Help コンポーネントへの動的アクセスクラス

	@author genta
	@date Jul. 5, 2001
*/
/*
	Copyright (C) 2001, genta
	Copyright (C) 2018-2022, Sakura Editor Organization

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

#ifndef SAKURA_CHTMLHELP_7003298B_3900_42FD_9A02_1BCD4E9A8546_H_
#define SAKURA_CHTMLHELP_7003298B_3900_42FD_9A02_1BCD4E9A8546_H_
#pragma once

#include <HtmlHelp.h>

#include "CDllHandler.h"

/*!
	@brief HtmpHelp動的ロード

	HTMLヘルプコンポーネントの動的ロードをサポートするクラス
*/
class CHtmlHelp : public CDllImp {
public:
	CHtmlHelp() noexcept;
	virtual ~CHtmlHelp() noexcept = default;

protected:
	//	HtmlHelp のEntry Point
	typedef decltype(::HtmlHelp)*	FnPtr_HtmlHelp;

	FnPtr_HtmlHelp		m_pfnHtmlHelp;

public:
	inline HWND WINAPI HtmlHelp(
		_In_opt_ HWND hwndCaller,
		_In_ LPCWSTR pszFile,
		_In_ UINT uCommand,
		_In_ DWORD_PTR dwData
	) const noexcept
	{
		return m_pfnHtmlHelp(hwndCaller, pszFile, uCommand, dwData);
	}

protected:
	virtual bool InitDllImp();
	virtual LPCWSTR GetDllNameImp(int nIndex);
};
#endif /* SAKURA_CHTMLHELP_7003298B_3900_42FD_9A02_1BCD4E9A8546_H_ */
