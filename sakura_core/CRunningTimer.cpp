//	$Id$
/************************************************************************

	CRunningTimer.cpp

    処理所要時間の計測クラス
	Copyright (C) 1998-2000, Norio Nakatani

    UPDATE:
    CREATE: 1998/3/6  新規作成


************************************************************************/

#include "CRunningTimer.h"
#include "global.h"
#include "debug.h"


int CRunningTimer::m_nNestCount = 0;

//	CRunningTimer::CRunningTimer()
//	{
//		Reset();
//		m_szText[0] = '\0';
//	//	m_nNestCount++;
///		return;
//	}

CRunningTimer::CRunningTimer( const char* pszText )
{
	Reset();
#ifdef _DEBUG
//	strcpy( m_szText, pszText );
//	if( '\0' != m_szText[0] ){
//		m_nNestCount++;
//	}
#endif
	return;
}


CRunningTimer::~CRunningTimer()
{
#ifdef _DEBUG
//	if( gm_ProfileOutput ){
//		if( '\0' != m_szText[0] ){
//			for( int i = 0; i < m_nNestCount; ++i ){
//				MYTRACE( "│" );
//			}
//			MYTRACE( "\"%s\", %d㍉秒\n", m_szText, GetTickCount() - m_nStartTime );
//		}
//	}
//	if( '\0' != m_szText[0] ){
//		m_nNestCount--;
//	}
#endif
	return;
}


void CRunningTimer::Reset()
{
	m_nStartTime = GetTickCount();
}


DWORD CRunningTimer::Read()
{
	return GetTickCount() - m_nStartTime;
}


/*[EOF]*/