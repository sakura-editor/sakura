//	$Id$
/************************************************************************

	CWaitCursor.cpp
	Copyright (C) 1998-2000, Norio Nakatani

************************************************************************/

#include "CWaitCursor.h"

CWaitCursor::CWaitCursor( HWND hWnd )
{
	SetCapture( hWnd );
	m_hCursor = ::LoadCursor( NULL, IDC_WAIT );
	m_hCursorOld = ::SetCursor( m_hCursor );
	return;
}



CWaitCursor::~CWaitCursor()
{
	ReleaseCapture();
	::SetCursor( m_hCursorOld );
	return;
}



/*[EOF]*/
