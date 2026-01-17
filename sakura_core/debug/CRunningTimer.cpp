/*!	@file
	@brief 処理所要時間の計測クラス

	デバッグ目的で用いる

	@author Norio Nakatani
	@date 1998/03/06  新規作成
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani
	Copyright (C) 2002, genta
	Copyright (C) 2018-2026, Sakura Editor Organization

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "debug/CRunningTimer.h"

int CRunningTimer::m_nNestCount = 0;
CRunningTimer::TimePoint CRunningTimer::m_initialTime = std::chrono::high_resolution_clock::now();

CRunningTimer::CRunningTimer( std::wstring_view name, OutputMode mode, OutputStyle style ) :
	m_timerName( name ),
	m_outputMode( mode ),
	m_outputStyle( style )
{
	if( m_timerName.length() == 0 && m_outputStyle == OutputStyle::Markdown ){
		// 字下げ位置がわかるように何か文字列を入れておく
		m_timerName = L"(no name)";
	}

	if( m_nNameOutputWidth < m_timerName.size() ){
		m_nNameOutputWidth = m_timerName.size();
	}

	m_nDepth = CRunningTimer::m_nNestCount++;
	if( m_nDepth == 0 ){
		OutputHeader();
	}
	Reset();
	WriteTraceInternal( m_startTime, TraceType::Enter );

	return;
}

CRunningTimer::~CRunningTimer()
{
	WriteTraceInternal( GetTime(), TraceType::ExitScope );
	FlushPendingTraces();
	if( m_nDepth == 0 ){
		OutputFooter();
	}
	m_nNestCount--;
	return;
}

void CRunningTimer::Reset()
{
	FlushPendingTraces();
	m_startTime = GetTime();
	m_lastTime = m_startTime;
}

uint32_t CRunningTimer::Read() const
{
	return (uint32_t)(GetElapsedTimeInSeconds( m_startTime, GetTime() ) * 1000.0);
}

/*!
	@date 2002.10.15 genta
*/
void CRunningTimer::WriteTrace( std::wstring_view msg )
{
	const auto currentTime = GetTime();
	WriteTraceInternal(currentTime, TraceType::Normal, msg);
}

void CRunningTimer::WriteTrace( int32_t n )
{
	const auto currentTime = GetTime();
	const auto msg = std::format(L"{}", n);
	WriteTraceInternal(currentTime, TraceType::Normal, msg);
}

CRunningTimer::TimePoint CRunningTimer::GetTime()
{
	return std::chrono::high_resolution_clock::now();
}

double CRunningTimer::GetElapsedTimeInSeconds( TimePoint from, TimePoint to )
{
	return std::chrono::duration<double>(to - from).count();
}

void CRunningTimer::WriteTraceInternal( TimePoint currentTime, TraceType traceType, std::wstring_view msg )
{
	if( m_outputMode == OutputMode::OnExitScope ){
		// 溜めておいて後でまとめて出力
		m_pendingTraces.emplace_back( TraceEntry( currentTime, traceType, msg ) );
	}else{
		// 溜めずにすぐ出力
		OutputTrace( currentTime, traceType, msg );
		m_lastTime = currentTime;
	}
}

void CRunningTimer::FlushPendingTraces()
{
	for( const auto& p : m_pendingTraces ){
		OutputTrace( p.m_timePoint, p.m_traceType, p.m_msg );
		m_lastTime = p.m_timePoint;
	}
	m_pendingTraces.clear();
}

void CRunningTimer::OutputHeader() const
{
	if( m_outputStyle == OutputStyle::Markdown ){
		Output(L"| timestamp (s) | {:<{}} | time (ms) | diff (ms) | message\n", L"name", m_nNameOutputWidth);
		Output(L"|--------------:|-{:-<{}}-|----------:|----------:|--------\n", L"", m_nNameOutputWidth);
	}else{
		// 従来形式では出力するものなし
	}
}

void CRunningTimer::OutputFooter() const
{
	if( m_outputStyle == OutputStyle::Markdown ){
		Output(L"\n");
	}else{
		// 従来形式では出力するものなし
	}
}

void CRunningTimer::OutputTrace( TimePoint currentTime, TraceType traceType, std::wstring_view msg ) const
{
	if( m_outputStyle == OutputStyle::Markdown ){
		if( traceType == TraceType::Enter ){
			msg = L"== Enter ==";
		}else if( traceType == TraceType::ExitScope ){
			msg = L"== Exit Scope ==";
		}else{
			//msg = msg
		}

		Output(L"| {:13.6f} | {:.{}}{:<{}} | {:9.3f} | {:9.3f} | {}\n",
			GetElapsedTimeInSeconds( CRunningTimer::m_initialTime, currentTime ),
			L"_ _ _ _ _ _ _ _ _ _ ", m_nDepth * 2, m_timerName, (m_nNameOutputWidth - (m_nDepth * 2)),
			GetElapsedTimeInSeconds( m_startTime, currentTime ) * 1000.0,
			GetElapsedTimeInSeconds( m_lastTime, currentTime ) * 1000.0,
			msg
		);
	}else{
		if( traceType == TraceType::Enter ){
			msg = L"Enter";
		}else if( traceType == TraceType::ExitScope ){
			msg = L"Exit Scope";
		}else{
			//msg = msg
		}

		if( traceType == TraceType::Enter ){
			Output(L"{:3d}:\"{}\" : {}\n",
				m_nDepth,
				m_timerName,
				msg
			);
		}else{
			Output(L"{:3d}:\"{}\", {}㍉秒 : {}\n",
				m_nDepth,
				m_timerName,
				(int)(GetElapsedTimeInSeconds( m_startTime, currentTime ) * 1000.0),
				msg
			);
		}
	}
}

void CRunningTimer::Output(const std::wstring& text) const
{
	::OutputDebugStringW(text.c_str());
}
