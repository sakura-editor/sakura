/*!	@file
	@brief �����v�J�[�\��

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
	���݂̃J�[�\����ۑ����A�J�[�\���������v�ɂ���
*/
CWaitCursor::CWaitCursor( HWND hWnd )
{
	SetCapture( hWnd );
	m_hCursor = ::LoadCursor( NULL, IDC_WAIT );
	m_hCursorOld = ::SetCursor( m_hCursor );
	return;
}



/*!
	�J�[�\���`������ɖ߂�
*/
CWaitCursor::~CWaitCursor()
{
	ReleaseCapture();
	::SetCursor( m_hCursorOld );
	return;
}



