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
	CRunningTimer t1( L"t1" );
	CRunningTimer t2( L"t2", CRunningTimer::OutputMode::OnExitScope );
	CRunningTimer t3( L"t3", CRunningTimer::OutputMode::OnWriteTrace, CRunningTimer::OutputStyle::Conventional );
	std::vector<CRunningTimer*> vt { &t1, &t2, &t3 };
	for( auto t : vt ){
		t->WriteTrace();
		t->WriteTrace( L"test" );
		t->WriteTrace( 1234567890 );
		t->WriteTraceFormat( L"%s:%d", L"test", 1234567890 );
		t->Read();
		t->Reset();
	}
}

/*!
 * @brief 長い文字列の入力
 */
TEST( CRunningTimer, LongString )
{
	auto str = L"012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789";
	CRunningTimer t( str );
	t.WriteTrace( str );
	t.WriteTrace( INT32_MAX );
	t.WriteTrace( INT32_MIN );
	t.WriteTraceFormat( L"%s%s%s", str, str, str );
}
