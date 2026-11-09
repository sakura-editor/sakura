/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "pch.h"
#include "debug/CRunningTimer.h"

/*!
 * @brief 基本動作
 */
TEST( CRunningTimer, Basic )
{
	CRunningTimer t0;
	CRunningTimer t1( LTEXT("t1") );
	CRunningTimer t2( LTEXT("t2"), CRunningTimer::OutputMode::OnExitScope );
	CRunningTimer t3( LTEXT("t3"), CRunningTimer::OutputMode::OnWriteTrace, CRunningTimer::OutputStyle::Conventional );
	std::vector<CRunningTimer*> vt { &t1, &t2, &t3 };
	for( auto t : vt ){
		t->WriteTrace();
		t->WriteTrace( LTEXT("test") );
		t->WriteTrace( 1234567890 );
		t->WriteTraceFormat( LTEXT("%s:%d"), LTEXT("test"), 1234567890 );
		t->Read();
		t->Reset();
	}
}

/*!
 * @brief 長い文字列の入力
 */
TEST( CRunningTimer, LongString )
{
	auto str = LTEXT("012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789");
	CRunningTimer t( str );
	t.WriteTrace( str );
	t.WriteTrace( INT32_MAX );
	t.WriteTrace( INT32_MIN );
	t.WriteTraceFormat( LTEXT("%s%s%s"), str, str, str );
}
