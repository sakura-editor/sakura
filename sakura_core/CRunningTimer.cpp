//	$Id$
/*!	@file
	@brief �������v���Ԃ̌v���N���X

	�f�o�b�O�ړI�ŗp����

	@author Norio Nakatani
	@date 1998/03/06  �V�K�쐬
	$Revision$
*/
/*
	Copyright (C) 1998-2001, Norio Nakatani

	This source code is designed for sakura editor.
	Please contact the copyright holder to use this code for other purpose.
*/

#include "CRunningTimer.h"
#include "global.h"
#include "debug.h"


int CRunningTimer::m_nNestCount = 0;

//	CRunningTimer::CRunningTimer()
//	{
//		Reset();
//		m_szText[0] = '\0';
//	//	m_nNestCount++;
///		return;
//	}

CRunningTimer::CRunningTimer( const char* pszText )
{
	Reset();
#ifdef _DEBUG
//	strcpy( m_szText, pszText );
//	if( '\0' != m_szText[0] ){
//		m_nNestCount++;
//	}
#endif
	return;
}


CRunningTimer::~CRunningTimer()
{
#ifdef _DEBUG
//	if( gm_ProfileOutput ){
//		if( '\0' != m_szText[0] ){
//			for( int i = 0; i < m_nNestCount; ++i ){
//				MYTRACE( "��" );
//			}
//			MYTRACE( "\"%s\", %d�_�b\n", m_szText, GetTickCount() - m_nStartTime );
//		}
//	}
//	if( '\0' != m_szText[0] ){
//		m_nNestCount--;
//	}
#endif
	return;
}


void CRunningTimer::Reset()
{
	m_nStartTime = GetTickCount();
}


DWORD CRunningTimer::Read()
{
	return GetTickCount() - m_nStartTime;
}


/*[EOF]*/
