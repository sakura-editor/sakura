//	$Id$
// sakura_core.cpp : DLL アプリケーション用のエントリ ポイントを定義します。
//	Copyright (C) 1998-2000, Norio Nakatani

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
SAKURA_CORE_API int nSakura_core=0;

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

