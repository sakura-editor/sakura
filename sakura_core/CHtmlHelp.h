//	$Id$
/*!	@file
	HTML Help コンポーネントへの動的アクセスクラス

	@author genta
	@date Jul. 5, 2001
*/
/*
	Copyright (C) 2001, genta
	
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

*/

#ifndef _SAKURA_HTMLHELP_H_
#define _SAKURA_HTMLHELP_H_

#include "CDllHandler.h"

/*!
	HTMLヘルプコンポーネントの動的ロードをサポートするクラス
*/
class SAKURA_CORE_API CHtmlHelp : public CDllHandler {
public:
	CHtmlhelp();
	virtual ~CHtmlHelp();

	//	HtmlHelp のEntry Point
	typedef HWND (WINAPI* Proc_HtmlHelp)(HWND, LPCSTR, UINT, DWORD);
	Proc_HtmlHelp HtmlHelp;

protected:
	virtual int InitDll(void);
	virtual char* GetDllName(char *);

};

#endif
