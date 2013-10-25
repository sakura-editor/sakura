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

// COsVersionInfo�̓���static�ϐ��̒�`
//	��������IsValidVersion()�ōs��
BOOL	 		COsVersionInfo::m_bSuccess;
OSVERSIONINFO	COsVersionInfo::m_cOsVersionInfo;
#ifdef USE_SSE2
bool			COsVersionInfo::m_bSSE2;
#endif
bool			COsVersionInfo::m_bWine;

/*!
	���������s��(�����̓_�~�[)
	�ďo�͊�{1��̂�
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

