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

#include "CHtmlHelp.h"

CHtmlHelp::~CHtmlHelp(void)
{}

/*!
	HTML Help のファイル名を渡す
*/
char *CHtmlHelp::GetDllName(char *)
{
	return "HHCTRL.OCX";
}

int CHtmlHelp::InitDll(void)
{
	if((HtmlHelp = (Proc_HtmlHelp)::GetProcAddress(GetInstance(),
#ifdef UNICODE
	"HtmlHelpW"
#else
	"HtmlHelpA"
#endif
	)) == NULL )
		return 1;

	return 0;
}
