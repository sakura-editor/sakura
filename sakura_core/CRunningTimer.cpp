//	$Id$
/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "stdafx.h"
#include "CRunningTimer.h"
#include "global.h"
#include "debug.h"


int CRunningTimer::m_nNestCount = 0;
#ifdef _DEBUG

CRunningTimer::CRunningTimer( const char* pszText )
{
	Reset();
	if( pszText != NULL )
		strcpy( m_szText, pszText );
	else
		m_szText[0] = '\0';
	m_nDeapth = m_nNestCount++;
	MYTRACE( "%3d:\"%s\" : Enter \n", m_nDeapth, m_szText );
	return;
}


CRunningTimer::~CRunningTimer()
{
	WriteTrace("Exit Scope");
	m_nNestCount--;
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

/*!
	@date 2002.10.15 genta
*/
void CRunningTimer::WriteTrace(const char* msg) const
{
	MYTRACE( "%3d:\"%s\", %d㍉秒 : %s\n", m_nDeapth, m_szText, GetTickCount() - m_nStartTime, msg );
}
#endif

/*[EOF]*/
