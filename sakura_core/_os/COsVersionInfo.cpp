/*!	@file
	@brief COsVersionInfo
*/
/*
	Copyright (C) 2013, novice

	This source code is designed for sakura editor.
	Please contact the copyright holders to use this code for other purpose.
*/

#include "StdAfx.h"
#include "_os/COsVersionInfo.h"
#include "util/RegKey.h"
#include "util/window.h"

// COsVersionInfoの内部static変数の定義
//	初期化はIsValidVersion()で行う
BOOL	 		COsVersionInfo::m_bSuccess;
OSVERSIONINFO	COsVersionInfo::m_cOsVersionInfo;
#ifdef USE_SSE2
bool			COsVersionInfo::m_bSSE2;
#endif
bool			COsVersionInfo::m_bWine;

/*!
	初期化を行う(引数はダミー)
	呼出は基本1回のみ
*/
COsVersionInfo::COsVersionInfo( bool pbStart )
{
	memset_raw( &m_cOsVersionInfo, 0, sizeof( m_cOsVersionInfo ) );
	m_cOsVersionInfo.dwOSVersionInfoSize = sizeof( m_cOsVersionInfo );
	m_bSuccess = ::GetVersionEx( &m_cOsVersionInfo );

	CRegKey reg;
	if( ERROR_SUCCESS == reg.Open(HKEY_CURRENT_USER, _T("Software\\Wine\\Debug"), KEY_READ)  ){
		m_bWine = true;
	}else{
		m_bWine = false;
	}
}

