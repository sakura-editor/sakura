//	$Id$
/*!	@file
	砂時計カーソル

	@author Norio Nakatani
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "CWaitCursor.h"

/*!
	現在のカーソルを保存し、カーソルを砂時計にする
*/
CWaitCursor::CWaitCursor( HWND hWnd )
{
	SetCapture( hWnd );
	m_hCursor = ::LoadCursor( NULL, IDC_WAIT );
	m_hCursorOld = ::SetCursor( m_hCursor );
	return;
}



/*!
	カーソル形状を元に戻す
*/
CWaitCursor::~CWaitCursor()
{
	ReleaseCapture();
	::SetCursor( m_hCursorOld );
	return;
}


/*[EOF]*/
