/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <MMSystem.h>
#include "debug/CRunningTimer.h"
#include "_main/global.h"

#ifdef _DEBUG

#pragma comment(lib, "winmm.lib")

int CRunningTimer::m_nNestCount = 0;

CRunningTimer::CRunningTimer( const char* pszText )
{
	Reset();
	if( pszText != NULL )
		strcpy( m_szText, pszText );
	else
		m_szText[0] = '\0';
	m_nDeapth = m_nNestCount++;
	MYTRACE( _T("%3d:\"%hs\" : Enter \n"), m_nDeapth, m_szText );
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
	m_nStartTime = timeGetTime();
}


DWORD CRunningTimer::Read()
{
	return timeGetTime() - m_nStartTime;
}

/*!
	@date 2002.10.15 genta
*/
void CRunningTimer::WriteTrace(const char* msg) const
{
	MYTRACE( _T("%3d:\"%hs\", %d㍉秒 : %hs\n"), m_nDeapth, m_szText, timeGetTime() - m_nStartTime, msg );
}
#endif


