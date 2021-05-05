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

CRunningTimer::TimePoint CRunningTimer::m_initialTime = std::chrono::high_resolution_clock::now();
int CRunningTimer::m_nNestCount = 0;

CRunningTimer::CRunningTimer( std::wstring_view name ) :
	m_timerName( name )
{
	Reset();

	m_nDepth = m_nNestCount++;
	OutputTrace( m_startTime, L"Enter", OutputTiming::Enter );
	return;
}

CRunningTimer::~CRunningTimer()
{
	OutputTrace( GetTime(), L"Exit Scope" );
	m_nNestCount--;
	return;
}

void CRunningTimer::Reset()
{
	m_startTime = GetTime();
}

DWORD CRunningTimer::Read()
{
	return (DWORD)(GetElapsedTimeInSeconds( m_startTime, GetTime() ) * 1000.0);
}

/*!
	@date 2002.10.15 genta
*/
void CRunningTimer::WriteTrace( std::wstring_view msg ) const
{
	OutputTrace( GetTime(), msg );
}

double CRunningTimer::GetElapsedTimeInSeconds( TimePoint from, TimePoint to )
{
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>( to - from ).count() / 1000.0 / 1000.0 / 1000.0;
}

CRunningTimer::TimePoint CRunningTimer::GetTime() const
{
	return std::chrono::high_resolution_clock::now();
}

void CRunningTimer::OutputTrace( TimePoint currentTime, std::wstring_view msg, OutputTiming timing ) const
{
	if( timing == OutputTiming::Enter )
	{
		Output( L"%3d:\"%s\" : %s \n", m_nDepth, m_timerName.c_str(), msg.data() );
	}
	else
	{
		Output( L"%3d:\"%s\", %d㍉秒 : %s\n", m_nDepth, m_timerName.c_str(), (int)(GetElapsedTimeInSeconds( m_startTime, currentTime ) * 1000.0), msg.data() );
	}
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
