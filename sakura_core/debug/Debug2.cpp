/*! @file */
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
