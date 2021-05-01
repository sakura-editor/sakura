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

#pragma comment(lib, "winmm.lib")

CRunningTimer::TimePoint CRunningTimer::m_initialTime = std::chrono::high_resolution_clock::now();
int CRunningTimer::m_nNestCount = 0;

CRunningTimer::CRunningTimer( std::wstring_view name, OutputStyle style ) :
	m_timerName( name ),
	m_nNameOutputWidthMin( 40 ),
	m_outputStyle( style )
{
	Reset();

	if( m_timerName.length() == 0 && m_outputStyle == OutputStyle::Markdown )
	{
		// 字下げ位置がわかるように何か文字列を入れておく
		m_timerName = L"(no name)";
	}

	m_nNameOutputWidth = m_nNameOutputWidthMin;
	if( m_nNameOutputWidth < m_timerName.size() )
	{
		m_nNameOutputWidth = m_timerName.size();
	}

	m_nDepth = m_nNestCount++;
	if( m_nDepth == 0 )
	{
		OutputHeader();
	}
	OutputTrace( m_startTime, TraceType::Enter );

	return;
}

CRunningTimer::~CRunningTimer()
{
	OutputTrace( GetTime(), TraceType::ExitScope );
	if( m_nDepth == 0 )
	{
		OutputFooter();
	}
	m_nNestCount--;
	return;
}

void CRunningTimer::Reset()
{
	m_startTime = GetTime();
	m_lastTime = m_startTime;
}

DWORD CRunningTimer::Read()
{
	return (DWORD)(GetElapsedTimeInSeconds( m_startTime, GetTime() ) * 1000.0);
}

/*!
	@date 2002.10.15 genta
*/
void CRunningTimer::WriteTrace( std::wstring_view msg )
{
	auto currentTime = GetTime();
	OutputTrace( currentTime, TraceType::Normal, msg );
	m_lastTime = currentTime;
}

double CRunningTimer::GetElapsedTimeInSeconds( TimePoint from, TimePoint to )
{
	return (double)std::chrono::duration_cast<std::chrono::nanoseconds>( to - from ).count() / 1000.0 / 1000.0 / 1000.0;
}

CRunningTimer::TimePoint CRunningTimer::GetTime() const
{
	return std::chrono::high_resolution_clock::now();
}

void CRunningTimer::OutputHeader() const
{
	if( m_outputStyle == OutputStyle::Markdown )
	{
		Output( L"| timestamp (s) | %-*s | time (ms) | diff (ms) | message\n", m_nNameOutputWidth, L"name" );
		Output( L"|--------------:|-%.*s-|----------:|----------:|--------\n", m_nNameOutputWidth, L"----------------------------------------------------------------------------------------------------" );
	}
	else
	{
		;
	}
}

void CRunningTimer::OutputFooter() const
{
	if( m_outputStyle == OutputStyle::Markdown )
	{
		Output( L"\n" );
	}
	else
	{
		;
	}
}

void CRunningTimer::OutputTrace( TimePoint currentTime, TraceType traceType, std::wstring_view msg ) const
{
	if( m_outputStyle == OutputStyle::Markdown )
	{
		msg =
			(traceType == TraceType::Enter) ? L"== Enter ==" :
			(traceType == TraceType::ExitScope) ? L"== Exit Scope ==" :
			msg;
		Output( L"| %13.6f | %.*s%-*s | %9.3f | %9.3f | %s\n",
			GetElapsedTimeInSeconds( m_initialTime, currentTime ),
			m_nDepth * 2, L"_ _ _ _ _ _ _ _ _ _ ", (m_nNameOutputWidth - (m_nDepth * 2)), m_timerName.c_str(),
			GetElapsedTimeInSeconds( m_startTime, currentTime ) * 1000.0,
			GetElapsedTimeInSeconds( m_lastTime, currentTime ) * 1000.0,
			msg.data() );
	}
	else
	{
		msg =
			(traceType == TraceType::Enter) ? L"Enter" :
			(traceType == TraceType::ExitScope) ? L"Exit Scope" :
			msg;
		if( traceType == TraceType::Enter )
		{
			Output( L"%3d:\"%s\" : %s\n",
				m_nDepth,
				m_timerName.c_str(),
				msg.data() );
		}
		else
		{
			Output( L"%3d:\"%s\", %d㍉秒 : %s\n",
				m_nDepth,
				m_timerName.c_str(),
				(int)(GetElapsedTimeInSeconds( m_startTime, currentTime ) * 1000.0),
				msg.data() );
		}
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
