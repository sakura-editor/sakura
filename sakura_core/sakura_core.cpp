//	$Id$
/*!	@file
	DLL アプリケーション用のエントリポイントを定義

	@author Norio Nakatani
	@date 1998/5/15 新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

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

#include "stdafx.h"
#include "sakura_core.h"

BOOL APIENTRY DllMain( HANDLE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}


// これはエクスポートされた変数の例です。
#ifndef __BORLANDC__
SAKURA_CORE_API int nSakura_core=0;
#else
SAKURA_CORE_API int nSakura_core;
#endif

// これはエクスポートされた関数の例です。
SAKURA_CORE_API int fnSakura_core(void)
{
	return 42;
}

// これはエクスポートされたクラスのコンストラクタです。
// クラスの定義については sakura_core.h を参照してください。
CSakura_core::CSakura_core()
{
	return;
}


/*[EOF]*/
