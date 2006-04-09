/*!	@file
	@brief マルチモニタ関数動的ロード

	マルチモニタ関数群への動的アクセスクラス

	@author genta
	@date May 01, 2004
*/
/*
	Copyright (C) 2004, genta

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
#include "stdafx.h"
#include <windows.h>
#include "CMultiMonitor.h"
#include "debug.h"

//-----------------------------------------
//	DLL 初期化関数
//-----------------------------------------

int CMultiMonitor::InitDll(void)
{
	//	staticにしてはいけない
	//	エントリー名はWindows SDKのMultiMon.hを参照
	
	const ImportTable table[] = {
		&m_MonitorFromWindow, "MonitorFromWindow",
		&m_GetMonitorInfo, "GetMonitorInfoA",
		NULL, 0
	};

	if( ! RegisterEntries(table) )
		return 1;

	return 0;
}

char* CMultiMonitor::GetDllName(char *)
{
	return "USER32";
}

CMultiMonitor::~CMultiMonitor()
{
}

//-----------------------------------------
//	Monitor情報取得関数
//-----------------------------------------
/*!
	指定したウィンドウのあるモニタの作業領域を得る

	@param hWnd [in] モニタを指定するためのウィンドウ
	@param rcDesktop [out]	モニタの範囲
	
	@return true:プライマリモニタ / false: それ以外のモニタ
*/
bool CMultiMonitor::GetMonitorWorkRect(HWND hWnd, LPRECT rcDesktop)
{
	if( MyInit() ){	//	マルチモニターサポート有り
		HMONITOR hMon = this->m_MonitorFromWindow( hWnd, MONITOR_DEFAULTTONEAREST );
		
		if( hMon != NULL ){
			MONITORINFO mi;
			::ZeroMemory( &mi, sizeof( mi ));
			mi.cbSize = sizeof( mi );

			if( this->m_GetMonitorInfo( hMon, &mi ) != 0 ){
				*rcDesktop = mi.rcWork;
				return ( mi.dwFlags == MONITORINFOF_PRIMARY ) ? true : false;
			}
			
		}
	}
	//	マルチモニターサポート無し．またはマルチモニタでうまくいかなかった場合．
	::SystemParametersInfo( SPI_GETWORKAREA, NULL, rcDesktop, 0 );
	return true;
}
