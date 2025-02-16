/*! @file */
/*
	Copyright (C) 2018-2022, Sakura Editor Organization

	SPDX-License-Identifier: Zlib
*/
#include "StdAfx.h"
#include "debug/Debug2.h"

void debug_exit()
{
	::exit( 1 );
}

void warning_point()
{
	::DebugBreak();
}
