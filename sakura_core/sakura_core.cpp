//	$Id$
/*!	@file
	DLL アプリケーション用のエントリポイントを定義

	@author Norio Nakatani
	@date 1998/5/15 新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
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
