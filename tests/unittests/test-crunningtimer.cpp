/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	This software is provided 'as-is', without any express or implied
	warranty. In no event will the authors be held liable for any damages
	arising from the use of this software.

	Permission is granted to anyone to use this software for any purpose,
	including commercial applications, and to alter it and redistribute it
	freely, subject to the following restrictions:

		1. The origin of this software must not be misrepresented;
		   you must not claim that you wrote the original software.
		   If you use this software in a product, an acknowledgment
		   in the product documentation would be appreciated but is
		   not required.

		2. Altered source versions must be plainly marked as such,
		   and must not be misrepresented as being the original software.

		3. This notice may not be removed or altered from any source
		   distribution.
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
