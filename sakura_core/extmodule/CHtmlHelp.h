/*!	@file
	@brief HtmpHelp動的ロード

	HTML Help コンポーネントへの動的アクセスクラス

	@author genta
	@date Jul. 5, 2001
*/
/*
	Copyright (C) 2001, genta

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

#ifndef _SAKURA_HTMLHELP_H_
#define _SAKURA_HTMLHELP_H_

#include "CDllHandler.h"


/*!
	@brief HtmpHelp動的ロード

	HTMLヘルプコンポーネントの動的ロードをサポートするクラス
*/
class CHtmlHelp : public CDllImp {
public:
	CHtmlHelp(){}
	virtual ~CHtmlHelp();

	//	HtmlHelp のEntry Point
	typedef HWND (WINAPI* Proc_HtmlHelp)(HWND, LPCTSTR, UINT, DWORD_PTR);
	Proc_HtmlHelp HtmlHelp;

protected:
	virtual bool InitDllImp();
	virtual LPCTSTR GetDllNameImp(int nIndex);
};

#endif



