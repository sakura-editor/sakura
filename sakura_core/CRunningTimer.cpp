//	$Id$
/*!	@file
	処理所要時間の計測クラス
	
	デバッグ目的で用いる
	
	@author Norio Nakatani
	@date 1998/03/06  新規作成
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