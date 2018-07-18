/*!	@file
	@brief 砂時計カーソル

	@author Norio Nakatani
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "StdAfx.h"
#include "CWaitCursor.h"

/*!
	現在のカーソルを保存し、カーソルを砂時計にする
*/
CWaitCursor::CWaitCursor( HWND hWnd, bool bEnable )
{
	m_bEnable = bEnable;
	if( ! bEnable ) return;
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
	if( m_bEnable ){
		ReleaseCapture();
		::SetCursor( m_hCursorOld );
	}
}



