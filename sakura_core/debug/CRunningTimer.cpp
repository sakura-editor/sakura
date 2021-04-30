/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2018-2021, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include <MMSystem.h>
#include "debug/CRunningTimer.h"
#include "_main/global.h"
#include "debug/Debug2.h"

#ifdef _DEBUG

#pragma comment(lib, "winmm.lib")

int CRunningTimer::m_nNestCount = 0;

CRunningTimer::CRunningTimer( const char* pszText )
{
	::QueryPerformanceFrequency( &m_nPerformanceFrequency );

	Reset();

	if( pszText != NULL )
		strcpy( m_szText, pszText );
	else
		m_szText[0] = '\0';
	m_nDeapth = m_nNestCount++;
	OutputTrace( m_nStartTime, "Enter", OutputTiming::Enter );
	return;
}

CRunningTimer::~CRunningTimer()
{
	OutputTrace( GetTime(), "Exit Scope" );
	m_nNestCount--;
	return;
}

void CRunningTimer::Reset()
{
	m_nStartTime = GetTime();
}

DWORD CRunningTimer::Read()
{
	return (DWORD)(GetTime() - m_nStartTime);
}

/*!
	@date 2002.10.15 genta
*/
void CRunningTimer::WriteTrace(const char* msg) const
{
	OutputTrace( GetTime(), msg );
}

void CRunningTimer::OutputTrace( double nCurrentTime, const char* msg, OutputTiming timing ) const
{
	if( timing == OutputTiming::Enter )
	{
		Output( L"%3d:\"%hs\" : %hs \n", m_nDeapth, m_szText, msg );
	}
	else
	{
		Output( L"%3d:\"%hs\", %d㍉秒 : %hs\n", m_nDeapth, m_szText, (int)(nCurrentTime - m_nStartTime), msg );
	}
}

double CRunningTimer::GetTime() const
{
	LARGE_INTEGER nCounter;
	::QueryPerformanceCounter( &nCounter );
	return (double)nCounter.QuadPart / m_nPerformanceFrequency.QuadPart * 1000.0;
}

void CRunningTimer::Output( std::wstring_view fmt, ... ) const
{
	va_list args;
	va_start( args, fmt );

	std::wstring str;
	vstrprintf( str, fmt.data(), args );

	va_end( args );

	OutputDebugStringW( str.data() );
}

#endif
