//	$Id$
/************************************************************************
	CDocLine.cpp
	Copyright (C) 1998-2000, Norio Nakatani
************************************************************************/

#include "CDocLine.h"

CDocLine::CDocLine()
{
	m_pPrev = NULL;
	m_pNext = NULL;
	m_pLine = NULL;
//	m_nType = 0;	/* �^�C�v 0=�ʏ� 1=�s�R�����g 2=�u���b�N�R�����g 3=�V���O���N�H�[�e�[�V���������� 4=�_�u���N�H�[�e�[�V���������� */
	m_bModify = TRUE;	/* �ύX�t���O */
//	m_enumEOLType = EOL_NONE;	/* ���s�R�[�h�̎�� */
//	m_nEOLLen = gm_pnEolLenArr[EOL_NONE];
//	m_nModifyCount = 32000;	/* �ύX�� */
	return;
}


CDocLine::~CDocLine()
{
	if( NULL != m_pLine ){
		delete m_pLine;
		m_pLine = NULL;
	}
	return;
}


/*[EOF]*/
